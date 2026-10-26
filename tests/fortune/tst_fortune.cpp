#include "fortuneengine.hpp"
#include "fortunelunar.hpp"

#include <QSet>
#include <QTest>

using namespace Fortune;

namespace {

auto profile(
    const QString &name = u"Ada"_s, const QDate &birthDate = QDate(1990, 6, 15), int birthHour = 6)
    -> Profile
{
    return Profile{name, birthDate, birthHour};
}

const QDateTime kMoment(QDate(2026, 10, 25), QTime(12, 0, 0));

} // namespace

class TestFortune : public QObject
{
    Q_OBJECT

private slots:
    void lunarNewYear_data();
    void lunarNewYear();
    void lunarLeapMonth_data();
    void lunarLeapMonth();
    void lunarRejectsOutsideRange();

    void castIsStable();
    void castChangesWithTheSecond();
    void birthDateChangesTheCast();
    void birthHourChangesTheBirthChart();
    void unknownHourEqualsZi();
    void nameChangesTheMovingLine();
    void castStaysInRange();
    void adviceIsDistinct();
    void invalidInputFails();

private:
    static auto requireReading(const Profile &p, const QDateTime &moment) -> Reading;
};

auto TestFortune::requireReading(const Profile &p, const QDateTime &moment) -> Reading
{
    const auto result = cast(p, moment);
    if (!result) {
        QTest::qFail(qPrintable(result.error()), __FILE__, __LINE__);
        return {};
    }
    return *result;
}

void TestFortune::lunarNewYear_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<int>("year");
    QTest::addColumn<int>("month");
    QTest::addColumn<int>("day");
    QTest::addColumn<bool>("leap");

    QTest::newRow("1901") << QDate(1901, 2, 19) << 1901 << 1 << 1 << false;
    QTest::newRow("2000") << QDate(2000, 2, 5) << 2000 << 1 << 1 << false;
    QTest::newRow("2020") << QDate(2020, 1, 25) << 2020 << 1 << 1 << false;
    QTest::newRow("2023") << QDate(2023, 1, 22) << 2023 << 1 << 1 << false;
    QTest::newRow("2024") << QDate(2024, 2, 10) << 2024 << 1 << 1 << false;
    QTest::newRow("2026") << QDate(2026, 10, 25) << 2026 << 9 << 16 << false;
}

void TestFortune::lunarNewYear()
{
    QFETCH(QDate, date);
    QFETCH(int, year);
    QFETCH(int, month);
    QFETCH(int, day);
    QFETCH(bool, leap);

    const auto lunar = toLunar(date);
    QVERIFY2(lunar.has_value(), "lunar conversion failed");

    QCOMPARE(lunar->year, year);
    QCOMPARE(lunar->month, month);
    QCOMPARE(lunar->day, day);
    QCOMPARE(lunar->isLeapMonth, leap);
}

void TestFortune::lunarLeapMonth_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<int>("month");
    QTest::addColumn<int>("day");

    QTest::newRow("2020-leap-4") << QDate(2020, 5, 23) << 4 << 1;
    QTest::newRow("2023-leap-2") << QDate(2023, 3, 22) << 2 << 1;
}

void TestFortune::lunarLeapMonth()
{
    QFETCH(QDate, date);
    QFETCH(int, month);
    QFETCH(int, day);

    const auto lunar = toLunar(date);
    QVERIFY2(lunar.has_value(), "lunar conversion failed");

    QCOMPARE(lunar->month, month);
    QCOMPARE(lunar->day, day);
    QVERIFY(lunar->isLeapMonth);
}

void TestFortune::lunarRejectsOutsideRange()
{
    QVERIFY(!isLunarDateSupported(QDate(1901, 2, 18)));
    QVERIFY(!isLunarDateSupported(QDate(2101, 1, 1)));
    QVERIFY(!toLunar(QDate(1899, 1, 1)).has_value());
    QVERIFY(!toLunar(QDate(2101, 1, 1)).has_value());
}

void TestFortune::castIsStable()
{
    const auto first = cast(profile(), kMoment);
    const auto second = cast(profile(), kMoment);

    QVERIFY2(first.has_value(), "first cast failed");
    QVERIFY2(second.has_value(), "second cast failed");
    QCOMPARE(*first, *second);
}

void TestFortune::castChangesWithTheSecond()
{
    const auto first = requireReading(profile(), kMoment);
    const auto second = requireReading(profile(), kMoment.addSecs(1));

    QVERIFY(!(first == second));
}

void TestFortune::birthDateChangesTheCast()
{
    QSet<int> hexagrams;
    for (int month = 1; month <= 12; ++month) {
        const Profile p = profile(u"Ada"_s, QDate(1990, month, 15), 6);
        const auto result = cast(p, kMoment);
        QVERIFY2(result.has_value(), "cast failed");
        hexagrams.insert(result->hexagram);
    }

    QVERIFY(hexagrams.size() > 1);
}

void TestFortune::birthHourChangesTheBirthChart()
{
    const auto first = birthChart(profile(u"Ada"_s, QDate(1990, 6, 15), 1));
    const auto second = birthChart(profile(u"Ada"_s, QDate(1990, 6, 15), 12));

    QVERIFY2(first.has_value(), "first chart failed");
    QVERIFY2(second.has_value(), "second chart failed");
    QVERIFY(!(*first == *second));
}

void TestFortune::unknownHourEqualsZi()
{
    const auto unknown = birthChart(profile(u"Ada"_s, QDate(1990, 6, 15), 0));
    const auto zi = birthChart(profile(u"Ada"_s, QDate(1990, 6, 15), 1));

    QVERIFY2(unknown.has_value(), "unknown hour chart failed");
    QVERIFY2(zi.has_value(), "zi hour chart failed");
    QCOMPARE(*unknown, *zi);
}

void TestFortune::nameChangesTheMovingLine()
{
    QSet<int> lines;
    for (int i = 0; i < 30; ++i) {
        const auto result = cast(profile(QString::number(i)), kMoment);
        QVERIFY2(result.has_value(), "cast failed");
        lines.insert(result->movingLine);
    }

    QVERIFY(lines.size() > 1);
}

void TestFortune::castStaysInRange()
{
    const Reading reading = requireReading(profile(), kMoment);

    QVERIFY(reading.hexagram >= 1 && reading.hexagram <= 64);
    QVERIFY(reading.changedHexagram >= 1 && reading.changedHexagram <= 64);
    QVERIFY(reading.movingLine >= 1 && reading.movingLine <= 6);
    QVERIFY(int(reading.grade) >= int(Grade::Unfavourable));
    QVERIFY(int(reading.grade) <= int(Grade::GreatFortune));
    QVERIFY(!reading.judgment.isEmpty());
    QVERIFY(!reading.headline.isEmpty());
    QVERIFY(!reading.reading.isEmpty());
    QVERIFY(reading.hexagram != reading.changedHexagram);
    QVERIFY(reading.luckyNumber >= 1 && reading.luckyNumber <= 10);
    QVERIFY(reading.luckyColor.startsWith(QLatin1Char('#')));
    QVERIFY(!reading.luckyElement.isEmpty());
    QCOMPARE(reading.doList.size(), 2);
    QCOMPARE(reading.avoidList.size(), 2);
}

void TestFortune::adviceIsDistinct()
{
    const Reading reading = requireReading(profile(), kMoment);

    QVERIFY(reading.doList.at(0) != reading.doList.at(1));
    QVERIFY(reading.avoidList.at(0) != reading.avoidList.at(1));
}

void TestFortune::invalidInputFails()
{
    QVERIFY(!cast(profile(u"Ada"_s, QDate(1899, 1, 1)), kMoment).has_value());
    QVERIFY(!cast(profile(u"Ada"_s, QDate(1990, 6, 15), 13), kMoment).has_value());
}

QTEST_GUILESS_MAIN(TestFortune)

#include "tst_fortune.moc"
