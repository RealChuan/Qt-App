#include "modaloverlay.hpp"

#include <utils/qtcassert.hpp>

#include <QEvent>
#include <QGraphicsBlurEffect>
#include <QWidget>

#include <algorithm>

namespace Widgets {

namespace {

// 对象名留给测试与调试定位，同时是下面这条内联样式的命中条件。
constexpr auto kMaskObjectName = "ModalMask";
// 遮挡颜色与主题无关：深浅两套配色下都该是"把内容压暗"，所以不进 base.css，
// 组件自己带着走，复制到别的程序里也不会因为少一条样式而失效。
constexpr auto kMaskColor = "rgba(0, 0, 0, 128)";
// 模糊半径：够糊住文字，又不至于把内容糊成一团色块。
constexpr int kBlurRadius = 12;

} // namespace

class ModalOverlay::ModalOverlayPrivate
{
public:
    explicit ModalOverlayPrivate(QWidget *target)
        : target(target)
    {}

    QWidget *target = nullptr;
    QWidget *mask = nullptr;
    QWidget *blurTarget = nullptr;
    QGraphicsBlurEffect *blur = nullptr;
    int inset = 0;
};

ModalOverlay::ModalOverlay(QWidget *target)
    : d_ptr(std::make_unique<ModalOverlayPrivate>(target))
{
    // 没有目标就没有遮挡层可言，传空指针是调用方的错：喊一声再退出。
    QTC_ASSERT(target, return);

    // 遮罩是目标的子控件：谁被遮挡，遮罩就归谁，和目标是窗口还是普通控件无关。
    d_ptr->mask = new QWidget(target);
    d_ptr->mask->setObjectName(QLatin1String(kMaskObjectName));
    // 纯 QWidget 默认不画背景，QSS 的 background 要配合这个属性才会生效。
    d_ptr->mask->setAttribute(Qt::WA_StyledBackground, true);
    d_ptr->mask->setStyleSheet(QStringLiteral("QWidget#%1 { background: %2; }")
                                   .arg(QLatin1String(kMaskObjectName), QLatin1String(kMaskColor)));
    d_ptr->mask->hide();

    target->installEventFilter(this);
}

ModalOverlay::~ModalOverlay() = default;

void ModalOverlay::setInset(int inset)
{
    d_ptr->inset = std::max(0, inset);
    if (maskIsVisible())
        updateMaskGeometry();
}

void ModalOverlay::setBlurTarget(QWidget *target)
{
    if (d_ptr->blurTarget == target)
        return;

    if (d_ptr->blurTarget) {
        // QWidget 接管 effect 的生命周期，摘掉时顺手把指针作废。
        d_ptr->blurTarget->setGraphicsEffect(nullptr);
        d_ptr->blur = nullptr;
    }

    d_ptr->blurTarget = target;
    // 空指针在这里是合法值，表示"只要遮罩不要模糊"，所以静默放过。
    if (!target)
        return;

    d_ptr->blur = new QGraphicsBlurEffect(target);
    d_ptr->blur->setBlurRadius(kBlurRadius);
    d_ptr->blur->setEnabled(false);
    target->setGraphicsEffect(d_ptr->blur);
}

void ModalOverlay::setMaskVisible(bool visible)
{
    // 目标为空时构造已经喊过一次，这里不再重复刷屏。
    if (!d_ptr->mask)
        return;

    if (!visible) {
        d_ptr->mask->hide();
        return;
    }

    updateMaskGeometry();
    d_ptr->mask->show();
    // 遮罩要盖在目标的所有子控件之上，包括标题栏。
    d_ptr->mask->raise();
}

void ModalOverlay::setBlurActive(bool active)
{
    if (d_ptr->blur)
        d_ptr->blur->setEnabled(active);
}

auto ModalOverlay::maskIsVisible() const -> bool
{
    return d_ptr->mask && !d_ptr->mask->isHidden();
}

auto ModalOverlay::blurIsActive() const -> bool
{
    return d_ptr->blur && d_ptr->blur->isEnabled();
}

bool ModalOverlay::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d_ptr->target) {
        switch (event->type()) {
        case QEvent::Resize:
        case QEvent::Show:
            if (maskIsVisible())
                updateMaskGeometry();
            break;
        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}

void ModalOverlay::updateMaskGeometry()
{
    if (!d_ptr->mask || !d_ptr->target)
        return;

    const QSize area = d_ptr->target->size();
    // 内缩量以目标短边的一半为上限：目标被缩得很小时遮罩不会算出负尺寸。
    const int inset = std::min(d_ptr->inset, std::min(area.width(), area.height()) / 2);
    d_ptr->mask->setGeometry(inset, inset, area.width() - 2 * inset, area.height() - 2 * inset);
}

} // namespace Widgets
