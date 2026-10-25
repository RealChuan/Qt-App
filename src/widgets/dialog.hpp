#pragma once

#include "appwindow.hpp"

#include <QDialog>

#include <memory>

class QKeyEvent;
class QPushButton;

namespace Widgets {

#ifndef Q_OS_MACOS
class WIDGETS_EXPORT Dialog : public AppWindow
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;

    // 阻塞直到 accept()/reject()/关闭，返回 QDialog::Accepted 或 Rejected。
    virtual auto exec() -> int;

signals:
    void accepted();
    void rejected();

public slots:
    virtual void accept();
    virtual void reject();

protected:
    // QDialog 的键盘语义不会跟着自绘壳出现，这里补上：
    // Esc 等于 reject()，回车点 QPushButton::setDefault(true) 标出来的那个按钮。
    void keyPressEvent(QKeyEvent *event) override;
    // 标题栏按钮、Alt+F4、程序化 close() 最终都落到这里：
    // 没有这条，exec() 的局部事件循环会一直挂着，窗口没了进程却卡死。
    void closeEvent(QCloseEvent *event) override;

private:
    void centerOnOwner();
    [[nodiscard]] auto defaultButton() const -> QPushButton *;

    class DialogPrivate;
    std::unique_ptr<DialogPrivate> d_ptr;
};
#else
// macOS 走原生窗口：装饰和窗口按钮都由系统画，对话框没有自绘标题栏。
// 和另一分支保持同样的签名，调用方不用写 #ifdef；做不到的能力在实现里告警，不静默吞掉。
class WIDGETS_EXPORT Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;

    // 系统对话框本来就没有最小化/最大化按钮，要求显示它们时只会告警。
    void setMinButtonVisible(bool visible);
    void setRestoreMaxButtonVisible(bool visible);

    void setCentralWidget(QWidget *widget);
    [[nodiscard]] auto shadowPadding() const -> int;
};
#endif

} // namespace Widgets
