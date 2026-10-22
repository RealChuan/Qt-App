#pragma once

#include <QColor>
#include <QWidget>

#include <memory>

namespace Danmaku {

// 铺在主窗口上的透明浮层：收到一条消息就生成一个 QLabel 并从右向左飘过。
class DanmakuOverlay final : public QWidget
{
    Q_OBJECT

public:
    explicit DanmakuOverlay(QWidget *parent);
    ~DanmakuOverlay() override;

    void push(const QString &text, const QColor &color);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    class DanmakuOverlayPrivate;
    std::unique_ptr<DanmakuOverlayPrivate> d_ptr;
};

} // namespace Danmaku
