#pragma once

#include "widgets_global.hpp"

#include <QMainWindow>
#include <QWidget>

#include <memory>

class QAction;
class QMenu;

namespace Widgets {

#ifndef Q_OS_MACOS
// 无边框窗体的公共外壳：标题栏、阴影、窗口按钮、拖动与四边拉伸。
// 只管"窗口长什么样、怎么动"，是否 show() 由使用方决定。
class WIDGETS_EXPORT AppWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AppWindow(QWidget *parent = nullptr);
    ~AppWindow() override;

    // false 时最大化与还原按钮一起隐藏。
    void setRestoreMaxButtonVisible(bool visible);
    void setMinButtonVisible(bool visible);

    // false 时四周不再响应拉伸：固定尺寸的外壳（模态对话框）用。
    void setResizable(bool resizable);

    // 主体改用卡片面 + 一圈描边：对话框没有页面层，直接铺画布色会和背后的
    // 主界面糊在一起。规则在 shell.css 的 #Frame[card="true"]。
    void setBodyAsCard(bool card);

    // 重复调用会替换上一个内容控件，不会叠加布局。
    void setCentralWidget(QWidget *centralWidget);

    // 命令的统一入口：Windows/Ubuntu 渲染成标题栏按钮（objectName 为
    // TitleBarButton，样式表按这个名字命中），macOS 收进系统顶栏的菜单里。
    // QAction 是跨平台的那一半，按钮和菜单项只是它的两种视图。
    void addChromeAction(QAction *action);

    // 顶级菜单：Windows/Ubuntu 进标题栏，macOS 进系统顶栏。
    void addMenu(QMenu *menu);

    // 标题栏里的自定义控件（搜索框、状态灯）。只有 Windows/Ubuntu 有这条标题栏，
    // macOS 上没有对应位置，调用会被忽略——需要跨平台请改用 addChromeAction()。
    void setTitleBarWidget(QWidget *widget);

    [[nodiscard]] auto shadowPadding() const -> int;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onShowMaximized();
    void onShowNormal();
    void onToggleMaximized();

private:
    [[nodiscard]] auto isInTitleBar(const QPoint &pos) const -> bool;
    void updateMaxRestoreButtons();
    void setupConnections();
    void retranslateUi();

    class AppWindowPrivate;
    std::unique_ptr<AppWindowPrivate> d_ptr;
};
#else
// macOS 走原生窗口：装饰和按钮都由系统画，菜单进系统顶栏，不需要自绘。
// 和另一分支保持同样的签名，调用方不用写 #ifdef；两边做不到的能力在实现里告警。
class WIDGETS_EXPORT AppWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AppWindow(QWidget *parent = nullptr);
    ~AppWindow() override;

    void setResizable(bool resizable);
    void setTitleBarWidget(QWidget *widget);
    void addChromeAction(QAction *action);
    void addMenu(QMenu *menu);
    // 原生装饰下主体本来就是系统画的窗口底色，这里无事可做。
    void setBodyAsCard(bool card);
    [[nodiscard]] auto shadowPadding() const -> int;
};
#endif

} // namespace Widgets
