#pragma once

#include "widgets_global.hpp"

#include <QMainWindow>
#include <QWidget>

#include <memory>

namespace Widgets {

#ifndef Q_OS_MACOS
// 无边框窗体的公共外壳：标题栏、阴影、最大化/还原/最小化按钮与拖拽。
// 只负责"窗口长什么样、怎么拖"，不负责什么时候显示——是否 show() 由使用方决定。
class WIDGETS_EXPORT MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() override;

    // false 时最大化与还原按钮一起隐藏。
    void setRestoreMaxButtonVisible(bool visible);
    void setMinButtonVisible(bool visible);

    // 重复调用会替换上一个内容/标题栏控件，不会叠加布局。
    void setCentralWidget(QWidget *centralWidget);
    void setTitleBar(QWidget *titleBar);

    [[nodiscard]] auto shadowPadding() const -> int;

    void setSizeGripVisible(bool visible);

signals:
    void aboutToClose();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void onShowMaximized();
    void onShowNormal();

private:
    [[nodiscard]] auto isInTitleBar(const QPoint &pos) const -> bool;
    void updateMaxRestoreButtons();
    void setupConnections();
    void retranslateUi();

    class MainWidgetPrivate;
    std::unique_ptr<MainWidgetPrivate> d_ptr;
};
#else
class WIDGETS_EXPORT MainWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {}

    void setTitleBar(QWidget *widget);
    [[nodiscard]] auto shadowPadding() const -> int { return 0; }

signals:
    void aboutToClose();
};
#endif

} // namespace Widgets
