#pragma once

#include "widgets_global.hpp"

#include <QObject>

#include <memory>

class QGraphicsBlurEffect;
class QWidget;

namespace Widgets {

// 盖在任意控件上的模态遮挡：半透明遮罩，外加可选的背景模糊。
// 遮罩是 target 的子控件，尺寸跟着 target 走（setInset() 用来避开窗口的阴影留白）；
// 模糊挂在单独的控件上——一个控件只能有一个 QGraphicsEffect，而且遮罩本身不该跟着一起糊。
class WIDGETS_EXPORT ModalOverlay : public QObject
{
    Q_OBJECT
public:
    explicit ModalOverlay(QWidget *target);
    ~ModalOverlay() override;

    // 遮罩四周内缩的像素数。目标窗口有阴影留白时用它盖住可见边框，而不是透明边距。
    void setInset(int inset);

    // 可选：把模糊挂到某个控件上。该控件原有的 effect 会被替换。
    void setBlurTarget(QWidget *target);

    void setMaskVisible(bool visible);
    void setBlurActive(bool active);

    [[nodiscard]] auto maskIsVisible() const -> bool;
    [[nodiscard]] auto blurIsActive() const -> bool;

protected:
    // 目标改尺寸时遮罩要跟着走，否则最大化/还原后会错位。
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void updateMaskGeometry();

    class ModalOverlayPrivate;
    std::unique_ptr<ModalOverlayPrivate> d_ptr;
};

} // namespace Widgets
