#include "hexagramboard.hpp"

#include "fortunecontent.hpp"

#include <QPainter>
#include <QPalette>
#include <QPointF>

#include <algorithm>

namespace Fortune {

namespace {

constexpr int kLineCount = 6;
constexpr qreal kHorizontalMargin = 26.0;
constexpr qreal kVerticalMargin = 18.0;
constexpr qreal kLineGap = 14.0;
constexpr qreal kBarHeightRatio = 0.62;
constexpr qreal kYinGapRatio = 0.30;
constexpr qreal kCornerRadius = 3.0;
constexpr qreal kMovingDotRadius = 3.5;
constexpr int kIdleAlpha = 110;
constexpr int kActiveAlpha = 215;
constexpr qreal kFullReveal = qreal(kLineCount);

} // namespace

class HexagramBoard::HexagramBoardPrivate
{
public:
    int upper = 1;
    int lower = 8;
    int movingLine = 0;
    bool isIdle = true;
    qreal revealProgress = kFullReveal;
};

HexagramBoard::HexagramBoard(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<HexagramBoardPrivate>())
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

HexagramBoard::~HexagramBoard() = default;

void HexagramBoard::setHexagram(int upper, int lower, int movingLine, bool isIdle)
{
    d_ptr->upper = std::clamp(upper, 1, 8);
    d_ptr->lower = std::clamp(lower, 1, 8);
    d_ptr->movingLine = std::clamp(movingLine, 0, kLineCount);
    d_ptr->isIdle = isIdle;
    update();
}

void HexagramBoard::clear()
{
    setHexagram(1, 8, 0, true);
    setRevealProgress(kFullReveal);
}

auto HexagramBoard::revealProgress() const -> qreal
{
    return d_ptr->revealProgress;
}

void HexagramBoard::setRevealProgress(qreal progress)
{
    const qreal clamped = std::clamp(progress, 0.0, kFullReveal);
    if (d_ptr->revealProgress == clamped)
        return;

    d_ptr->revealProgress = clamped;
    update();
}

auto HexagramBoard::sizeHint() const -> QSize
{
    return {220, 240};
}

void HexagramBoard::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF area
        = QRectF(rect())
              .adjusted(kHorizontalMargin, kVerticalMargin, -kHorizontalMargin, -kVerticalMargin);
    if (area.width() <= 0.0 || area.height() <= 0.0)
        return;

    const qreal lineHeight = (area.height() - kLineGap * (kLineCount - 1)) / kLineCount;
    const qreal barHeight = lineHeight * kBarHeightRatio;
    const qreal barWidth = area.width();

    const QPalette &palette = this->palette();
    const QColor active = palette.color(QPalette::WindowText);
    const QColor highlight = palette.color(QPalette::Highlight);

    const int lowerMask = trigramLineMask(d_ptr->lower);
    const int upperMask = trigramLineMask(d_ptr->upper);

    for (int index = 0; index < kLineCount; ++index) {
        const qreal progress = std::clamp(d_ptr->revealProgress - index, 0.0, 1.0);
        if (progress <= 0.0)
            continue;

        const bool isLower = index < 3;
        const bool isYang = isLower ? ((lowerMask >> index) & 1) != 0
                                    : ((upperMask >> (index - 3)) & 1) != 0;
        const bool isMoving = !d_ptr->isIdle && d_ptr->movingLine == index + 1;

        QColor color = isMoving ? highlight : active;
        color.setAlpha(isMoving ? 255 : (d_ptr->isIdle ? kIdleAlpha : kActiveAlpha));

        const qreal slotBottom = area.bottom() - index * (lineHeight + kLineGap);
        const qreal top = slotBottom - lineHeight + (lineHeight - barHeight) / 2.0;
        const QRectF slot(area.left(), top, barWidth, barHeight);

        painter.save();
        const qreal visibleWidth = barWidth * progress;
        painter.setClipRect(
            QRectF(slot.center().x() - visibleWidth / 2.0, slot.top(), visibleWidth, barHeight));

        painter.setPen(Qt::NoPen);
        painter.setBrush(color);

        if (isYang) {
            painter.drawRoundedRect(slot, kCornerRadius, kCornerRadius);
        } else {
            const qreal gap = barWidth * kYinGapRatio;
            const qreal segmentWidth = (barWidth - gap) / 2.0;
            painter.drawRoundedRect(
                QRectF(slot.left(), slot.top(), segmentWidth, barHeight),
                kCornerRadius,
                kCornerRadius);
            painter.drawRoundedRect(
                QRectF(slot.right() - segmentWidth, slot.top(), segmentWidth, barHeight),
                kCornerRadius,
                kCornerRadius);
        }

        painter.restore();

        if (isMoving) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(highlight);
            painter.drawEllipse(
                QPointF(area.right() - kMovingDotRadius, slot.center().y()),
                kMovingDotRadius,
                kMovingDotRadius);
        }
    }
}

} // namespace Fortune
