#include "fortunesharing.hpp"

#include "fortunecontent.hpp"
#include "fortunestrings.hpp"
#include "hexagramboard.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>

namespace Fortune {

namespace {

constexpr int kCardWidth = 880;
constexpr int kBoardWidth = 220;
constexpr int kBoardHeight = 240;
constexpr int kOuterMargin = 24;
constexpr int kCardMargin = 32;
constexpr int kColumnGap = 28;
constexpr qreal kRenderScale = 2.0;

auto gradeText(Grade grade) -> QString
{
    return gradeName(grade);
}

auto trigramPair(int upper, int lower) -> QString
{
    return FortuneStrings::tr("%1 over %2").arg(trigramImage(upper), trigramImage(lower));
}

} // namespace

auto shareText(const Reading &reading) -> QString
{
    const HexagramInfo &info = hexagram(reading.hexagram);

    QStringList lines;
    lines << FortuneStrings::tr("Hexagram %1: %2 (%3)")
                 .arg(info.number)
                 .arg(hexagramName(reading.hexagram))
                 .arg(trigramPair(info.upper, info.lower));
    lines << FortuneStrings::tr("Moving line: %1").arg(reading.movingLine);
    lines << FortuneStrings::tr("Grade: %1").arg(gradeText(reading.grade));
    lines << QString();
    lines << reading.judgment;
    lines << QString();
    lines << reading.reading;
    lines << QString();
    lines << FortuneStrings::tr("Do: %1").arg(reading.doList.join(u", "_s));
    lines << FortuneStrings::tr("Avoid: %1").arg(reading.avoidList.join(u", "_s));
    lines << FortuneStrings::tr("Lucky element: %1 · number %2")
                 .arg(reading.luckyElement)
                 .arg(reading.luckyNumber);
    lines << FortuneStrings::tr("Birth hexagram: %1 · Changed: %2")
                 .arg(reading.birthHexagram)
                 .arg(hexagramName(reading.changedHexagram));
    lines << QString();
    lines << FortuneStrings::tr("Cast at %1").arg(reading.castAt.toString(u"yyyy-MM-dd HH:mm"_s));
    lines << FortuneStrings::tr(
        "For entertainment only. A simplified Plum Blossom reading, "
        "not advice.");

    return lines.join(QLatin1Char('\n'));
}

auto renderShareImage(const Reading &reading) -> QImage
{
    const HexagramInfo &info = hexagram(reading.hexagram);

    QWidget outer;
    outer.setObjectName(u"FortuneShareOuter"_s);
    outer.setAttribute(Qt::WA_DontShowOnScreen);

    auto *outerLayout = new QVBoxLayout(&outer);
    outerLayout->setContentsMargins(kOuterMargin, kOuterMargin, kOuterMargin, kOuterMargin);

    auto *card = new QFrame(&outer);
    card->setObjectName(u"Card"_s);
    card->setFixedWidth(kCardWidth);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(kCardMargin, kCardMargin, kCardMargin, kCardMargin);
    cardLayout->setSpacing(12);

    auto *headerRow = new QHBoxLayout;
    headerRow->setContentsMargins(0, 0, 0, 0);
    auto *titleLabel = new QLabel(FortuneStrings::tr("Cyber Fortune"), card);
    titleLabel->setObjectName(u"CardTitle"_s);
    auto *dateLabel = new QLabel(reading.castAt.toString(u"yyyy-MM-dd HH:mm"_s), card);
    dateLabel->setObjectName(u"HeroMeta"_s);
    headerRow->addWidget(titleLabel);
    headerRow->addStretch();
    headerRow->addWidget(dateLabel);

    auto *divider = new QFrame(card);
    divider->setObjectName(u"Divider"_s);
    divider->setFrameShape(QFrame::NoFrame);

    auto *board = new HexagramBoard(card);
    board->setFixedSize(kBoardWidth, kBoardHeight);
    board->setHexagram(info.upper, info.lower, reading.movingLine, false);
    board->setRevealProgress(6.0);

    auto *infoPanel = new QWidget(card);
    auto *infoLayout = new QVBoxLayout(infoPanel);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(6);

    auto *nameLabel = new QLabel(
        FortuneStrings::tr("Hexagram %1: %2").arg(info.number).arg(hexagramName(reading.hexagram)),
        infoPanel);
    nameLabel->setObjectName(u"HeroTitle"_s);
    nameLabel->setWordWrap(true);

    auto *trigramLabel = new QLabel(trigramPair(info.upper, info.lower), infoPanel);
    trigramLabel->setObjectName(u"HeroMeta"_s);

    auto *gradeLabel
        = new QLabel(u"%1 · %2"_s.arg(gradeText(reading.grade), reading.headline), infoPanel);
    gradeLabel->setObjectName(u"CardTitle"_s);
    gradeLabel->setWordWrap(true);

    auto *judgmentLabel = new QLabel(reading.judgment, infoPanel);
    judgmentLabel->setObjectName(u"CardHint"_s);
    judgmentLabel->setWordWrap(true);

    auto *readingLabel = new QLabel(reading.reading, infoPanel);
    readingLabel->setObjectName(u"CardHint"_s);
    readingLabel->setWordWrap(true);

    auto *doLabel
        = new QLabel(FortuneStrings::tr("Do: %1").arg(reading.doList.join(u" · "_s)), infoPanel);
    doLabel->setObjectName(u"CardHint"_s);
    doLabel->setWordWrap(true);

    auto *avoidLabel = new QLabel(
        FortuneStrings::tr("Avoid: %1").arg(reading.avoidList.join(u" · "_s)), infoPanel);
    avoidLabel->setObjectName(u"CardHint"_s);
    avoidLabel->setWordWrap(true);

    auto *luckyLabel = new QLabel(
        FortuneStrings::tr("Element %1 · lucky number %2")
            .arg(reading.luckyElement)
            .arg(reading.luckyNumber),
        infoPanel);
    luckyLabel->setObjectName(u"CardHint"_s);

    auto *birthLabel = new QLabel(
        FortuneStrings::tr("Birth hexagram: %1 · Changed: %2")
            .arg(reading.birthHexagram)
            .arg(hexagramName(reading.changedHexagram)),
        infoPanel);
    birthLabel->setObjectName(u"HeroMeta"_s);
    birthLabel->setWordWrap(true);

    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(trigramLabel);
    infoLayout->addWidget(gradeLabel);
    infoLayout->addWidget(judgmentLabel);
    infoLayout->addWidget(readingLabel);
    infoLayout->addWidget(doLabel);
    infoLayout->addWidget(avoidLabel);
    infoLayout->addWidget(luckyLabel);
    infoLayout->addWidget(birthLabel);
    infoLayout->addStretch();

    auto *contentRow = new QHBoxLayout;
    contentRow->setContentsMargins(0, 0, 0, 0);
    contentRow->setSpacing(kColumnGap);
    contentRow->addWidget(board, 0, Qt::AlignTop);
    contentRow->addWidget(infoPanel, 1);

    auto *footerLabel = new QLabel(
        FortuneStrings::tr("For entertainment only. A simplified Plum Blossom reading, not advice."),
        card);
    footerLabel->setObjectName(u"Placeholder"_s);
    footerLabel->setWordWrap(true);

    cardLayout->addLayout(headerRow);
    cardLayout->addWidget(divider);
    cardLayout->addLayout(contentRow);
    cardLayout->addWidget(footerLabel);

    outerLayout->addWidget(card);

    outer.ensurePolished();
    outer.show();
    QCoreApplication::processEvents();
    outer.adjustSize();

    const QSize logicalSize = outer.size();
    if (logicalSize.isEmpty())
        return {};

    QImage image(logicalSize * kRenderScale, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(kRenderScale);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    outer.render(&painter);

    return image;
}

} // namespace Fortune
