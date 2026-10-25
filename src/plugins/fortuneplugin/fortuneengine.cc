#include "fortuneengine.hpp"

#include "fortunelunar.hpp"
#include "fortunestrings.hpp"

#include <QCoreApplication>
#include <QCryptographicHash>

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

namespace Fortune {

namespace {

constexpr int kTrigramCount = 8;
constexpr int kLineCount = 6;

auto digest(const QByteArray &payload) -> QByteArray
{
    return QCryptographicHash::hash(payload, QCryptographicHash::Sha256);
}

auto wordAt(const QByteArray &hash, int index) -> std::uint32_t
{
    const int offset = (index * 4) % (hash.size() - 3);
    return (std::uint32_t(std::uint8_t(hash[offset])) << 24)
           | (std::uint32_t(std::uint8_t(hash[offset + 1])) << 16)
           | (std::uint32_t(std::uint8_t(hash[offset + 2])) << 8)
           | std::uint32_t(std::uint8_t(hash[offset + 3]));
}

auto hourBranch(int hour) -> int
{
    const int branch = ((hour + 1) / 2 + 1) % 12;
    return branch == 0 ? 12 : branch;
}

auto generates(Element from, Element to) -> bool
{
    switch (from) {
    case Element::Wood:
        return to == Element::Fire;
    case Element::Fire:
        return to == Element::Earth;
    case Element::Earth:
        return to == Element::Metal;
    case Element::Metal:
        return to == Element::Water;
    case Element::Water:
        return to == Element::Wood;
    }
    return false;
}

auto controls(Element from, Element to) -> bool
{
    switch (from) {
    case Element::Wood:
        return to == Element::Earth;
    case Element::Earth:
        return to == Element::Water;
    case Element::Water:
        return to == Element::Fire;
    case Element::Fire:
        return to == Element::Metal;
    case Element::Metal:
        return to == Element::Wood;
    }
    return false;
}

auto relationGrade(Element use, Element body) -> Grade
{
    if (generates(use, body))
        return Grade::GreatFortune;
    if (use == body)
        return Grade::Good;
    if (controls(body, use))
        return Grade::Fair;
    if (generates(body, use))
        return Grade::Caution;
    return Grade::Unfavourable;
}

auto relationHeadline(Element use, Element body) -> QString
{
    if (generates(use, body))
        return FortuneStrings::tr("Support arrives from outside.");
    if (use == body)
        return FortuneStrings::tr("Steady ground, no sudden move.");
    if (controls(body, use))
        return FortuneStrings::tr("You hold the initiative.");
    if (generates(body, use))
        return FortuneStrings::tr("The effort is yours to spend.");
    return FortuneStrings::tr("Outside pressure sets the tone.");
}

auto relationSentence(Element use, Element body) -> QString
{
    const QString useName = elementName(use);
    const QString bodyName = elementName(body);

    if (generates(use, body)) {
        return FortuneStrings::tr(
                   "The %1 element feeds the %2 element: help comes from the situation itself.")
            .arg(useName, bodyName);
    }
    if (use == body) {
        return FortuneStrings::tr("Both trigrams are %1: the day is steady rather than dramatic.")
            .arg(useName);
    }
    if (controls(body, use)) {
        return FortuneStrings::tr(
                   "The %1 element holds the %2 element in check: the initiative stays "
                   "with you, and so does the work.")
            .arg(bodyName, useName);
    }
    if (generates(body, use)) {
        return FortuneStrings::tr(
                   "The %1 element drains into the %2 element: energy leaves faster "
                   "than it returns.")
            .arg(bodyName, useName);
    }
    return FortuneStrings::tr(
               "The %1 element presses on the %2 element: outside pressure sets the tone, so guard "
               "your margin.")
        .arg(useName, bodyName);
}

auto linePositionName(int line) -> QString
{
    static constexpr std::array<const char *, kLineCount> kNames{
        QT_TRANSLATE_NOOP("FortuneStrings", "first"),
        QT_TRANSLATE_NOOP("FortuneStrings", "second"),
        QT_TRANSLATE_NOOP("FortuneStrings", "third"),
        QT_TRANSLATE_NOOP("FortuneStrings", "fourth"),
        QT_TRANSLATE_NOOP("FortuneStrings", "fifth"),
        QT_TRANSLATE_NOOP("FortuneStrings", "sixth"),
    };
    return FortuneStrings::tr(kNames[std::clamp(line, 1, kLineCount) - 1]);
}

auto pickTwo(std::span<const char *const> pool, const QByteArray &seed, int offset) -> QStringList
{
    const int count = int(pool.size());
    if (count < 2)
        return {};

    const int first = int(std::uint8_t(seed[offset % seed.size()])) % count;
    const int step = 1 + int(std::uint8_t(seed[(offset + 1) % seed.size()])) % (count - 1);
    const int second = (first + step) % count;

    return {
        QCoreApplication::translate("FortuneContent", pool[size_t(first)]),
        QCoreApplication::translate("FortuneContent", pool[size_t(second)])};
}

auto chartFromLunar(const LunarDate &birth, int birthHour) -> BirthChart
{
    const int base = birth.yearBranch + birth.month + birth.day;
    const int hour = birthHour == 0 ? 1 : birthHour;
    const int upper = ((base - 1) % kTrigramCount) + 1;
    const int lower = ((base + hour - 1) % kTrigramCount) + 1;
    const int moving = ((base + hour - 1) % kLineCount) + 1;

    return BirthChart{hexagramFor(upper, lower), moving, trigram(lower).element};
}

} // namespace

auto birthChart(const Profile &profile) -> Utils::Result<BirthChart>
{
    if (profile.birthHour < 0 || profile.birthHour > 12)
        return std::unexpected(FortuneStrings::tr("Birth hour must be between 0 and 12."));

    const auto lunar = toLunar(profile.birthDate);
    if (!lunar)
        return std::unexpected(lunar.error());

    return chartFromLunar(*lunar, profile.birthHour);
}

auto cast(const Profile &profile, const QDateTime &moment) -> Utils::Result<Reading>
{
    if (!moment.isValid())
        return std::unexpected(FortuneStrings::tr("Cast moment is invalid."));

    if (profile.birthHour < 0 || profile.birthHour > 12)
        return std::unexpected(FortuneStrings::tr("Birth hour must be between 0 and 12."));

    const auto birth = toLunar(profile.birthDate);
    if (!birth)
        return std::unexpected(birth.error());

    const auto today = toLunar(moment.date());
    if (!today)
        return std::unexpected(today.error());

    const BirthChart birthInfo = chartFromLunar(*birth, profile.birthHour);

    const QByteArray nameBytes = profile.name.toUtf8();
    const auto external = wordAt(digest(nameBytes), 1) % kLineCount;

    const int birthBase = birth->yearBranch + birth->month + birth->day;
    const int dayBase = today->yearBranch + today->month + today->day;
    const int hour = hourBranch(moment.time().hour());
    const int minute = moment.time().minute();
    const int second = moment.time().second();

    const int upper = ((birthBase + dayBase - 1) % kTrigramCount) + 1;
    const int lower = ((birthBase + dayBase + hour + minute + second - 1) % kTrigramCount) + 1;
    const int moving = ((birthBase + dayBase + hour + minute + second + int(external) - 1)
                        % kLineCount)
                       + 1;

    const int number = hexagramFor(upper, lower);
    const int useTrigram = moving <= 3 ? lower : upper;
    const int bodyTrigram = moving <= 3 ? upper : lower;
    const Element useElement = trigram(useTrigram).element;
    const Element bodyElement = trigram(bodyTrigram).element;

    Grade grade = relationGrade(useElement, bodyElement);
    if (birthInfo.element == bodyElement && grade != Grade::GreatFortune)
        grade = Grade(std::min(int(grade) + 1, int(Grade::GreatFortune)));

    const int lines = trigramLineMask(lower) | (trigramLineMask(upper) << 3);
    const int changedLines = lines ^ (1 << (moving - 1));
    const int changedHexagram
        = hexagramFor(trigramFromLineMask(changedLines >> 3), trigramFromLineMask(changedLines));

    const bool isYang = ((lines >> (moving - 1)) & 1) != 0;

    QByteArray seedInput = nameBytes;
    seedInput += '|';
    seedInput += profile.birthDate.toString(Qt::ISODate).toUtf8();
    seedInput += '|';
    seedInput += moment.toString(Qt::ISODate).toUtf8();
    const QByteArray seed = digest(seedInput);

    const std::span<const int> numbers = elementLuckyNumbers(bodyElement);
    const int luckyNumber = numbers[size_t(std::uint8_t(seed[0]) % numbers.size())];

    const QString birthSentence
        = birthInfo.element == bodyElement
              ? FortuneStrings::tr(
                    "Your birth hexagram is %1; its element, %2, matches your position "
                    "and steadies the reading.")
                    .arg(birthInfo.hexagram)
                    .arg(elementName(birthInfo.element))
              : FortuneStrings::tr(
                    "Your birth hexagram is %1; its element, %2, sits apart from "
                    "today's position.")
                    .arg(birthInfo.hexagram)
                    .arg(elementName(birthInfo.element));

    Reading reading;
    reading.castAt = moment;
    reading.hexagram = number;
    reading.movingLine = moving;
    reading.changedHexagram = changedHexagram;
    reading.grade = grade;
    reading.judgment = hexagramJudgment(number);
    reading.headline = relationHeadline(useElement, bodyElement);
    reading.reading = FortuneStrings::tr(
                          "The moving line is the %1, %2. The %3 acts as the matter; %4 is your "
                          "position. %5")
                          .arg(linePositionName(moving))
                          .arg(isYang ? FortuneStrings::tr("yang") : FortuneStrings::tr("yin"))
                          .arg(trigramName(useTrigram))
                          .arg(trigramName(bodyTrigram))
                          .arg(relationSentence(useElement, bodyElement))
                      + QStringLiteral("\n\n") + birthSentence + QStringLiteral("\n\n")
                      + FortuneStrings::tr(
                            "With the moving line turned, the reading becomes %1: %2.")
                            .arg(changedHexagram)
                            .arg(hexagramName(changedHexagram));
    reading.doList = pickTwo(doAdvice(grade), seed, 1);
    reading.avoidList = pickTwo(avoidAdvice(grade), seed, 3);
    reading.luckyNumber = luckyNumber;
    reading.luckyColor = QString::fromLatin1(elementColorHex(bodyElement));
    reading.luckyElement = elementName(bodyElement);
    reading.birthHexagram = birthInfo.hexagram;

    return reading;
}

} // namespace Fortune
