#pragma once

#include <QWidget>

#include <memory>

namespace ColorPicker {

// 独立工具窗：跟随鼠标、显示放大镜，点击复制 HEX。
// 演示插件如何拥有自己的置顶窗口与全局轮询，而不占用主窗口的页面空间。
class ColorPickerWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit ColorPickerWidget(QWidget *parent = nullptr);
    ~ColorPickerWidget() override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void onPollTimeout();

    class ColorPickerWidgetPrivate;
    std::unique_ptr<ColorPickerWidgetPrivate> d_ptr;
};

} // namespace ColorPicker
