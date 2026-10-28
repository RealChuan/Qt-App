#include "danmakuoverlay.hpp"

#include <QEvent>
#include <QLabel>
#include <QPropertyAnimation>

using namespace Qt::StringLiterals;

namespace Danmaku {

namespace {
constexpr int kLaneHeight = 34;
constexpr int kSpeed = 220;    // px / s
constexpr int kTopOffset = 56; // 让开自定义标题栏
} // namespace

class DanmakuOverlay::DanmakuOverlayPrivate
{
public:
    explicit DanmakuOverlayPrivate(DanmakuOverlay *q)
        : q_ptr(q)
    {}

    DanmakuOverlay *q_ptr = nullptr;
    int nextLane = 0;
};

DanmakuOverlay::DanmakuOverlay(QWidget *parent)
    : QWidget(parent)
    , d_ptr(std::make_unique<DanmakuOverlayPrivate>(this))
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setGeometry(parent->rect());
    parent->installEventFilter(this);
    raise();
}

DanmakuOverlay::~DanmakuOverlay() = default;

bool DanmakuOverlay::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget() && event->type() == QEvent::Resize) {
        setGeometry(parentWidget()->rect());
        raise();
    }
    return QWidget::eventFilter(watched, event);
}

void DanmakuOverlay::push(const QString &text, const QColor &color)
{
    auto *label = new QLabel(text, this);
    // 弹幕浮层：字号跟正文一档，底色固定半透明黑（背景可能是任何页面）。
    label->setStyleSheet(
        u"QLabel { color: %1; background: rgba(0, 0, 0, 110);"
        u" border-radius: 8px; padding: 3px 10px;"
        u" font-size: 14px; font-weight: 600; }"_s.arg(
            (color.isValid() ? color : QColor(255, 255, 255)).name()));

    label->adjustSize();
    const int lanes = qMax(1, (height() - kTopOffset) / kLaneHeight);
    const int y = kTopOffset + (d_ptr->nextLane++ % lanes) * kLaneHeight;

    label->move(width(), y);
    label->show();

    auto *animation = new QPropertyAnimation(label, "pos", label);
    animation->setStartValue(QPoint(width(), y));
    animation->setEndValue(QPoint(-label->width(), y));
    animation->setDuration(qMax(1200, int((width() + label->width()) * 1000 / kSpeed)));
    animation->setEasingCurve(QEasingCurve::Linear);
    connect(animation, &QPropertyAnimation::finished, label, &QLabel::deleteLater);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace Danmaku
