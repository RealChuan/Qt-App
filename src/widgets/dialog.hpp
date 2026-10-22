#pragma once

#include "mainwidget.hpp"

#include <QDialog>

#include <memory>

namespace Widgets {

#ifndef Q_OS_MACOS
// 模态对话框：在 MainWidget 外壳上把最大化/还原/尺寸手柄关掉，
// exec() 用局部事件循环实现（非 macOS 侧没有 QDialog）。
class WIDGETS_EXPORT Dialog : public MainWidget
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;

    // 阻塞直到 accept()/reject()/关闭，返回 QDialog::Accepted 或 Rejected。
    auto exec() -> int;

signals:
    void accepted();
    void rejected();

public slots:
    void accept();
    void reject();

private slots:
    void onClosed();
    void onMoveParentCenter();

protected:
    // Alt+F4 / 系统关闭都走 close()，不会发 aboutToClose；
    // 没有这条，exec() 的局部事件循环会一直挂着，窗口没了进程却卡死。
    void closeEvent(QCloseEvent *event) override;

private:
    void setupConnections();

    class DialogPrivate;
    std::unique_ptr<DialogPrivate> d_ptr;
};
#else
class WIDGETS_EXPORT Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
    }

    void setMinButtonVisible(bool) {}
    void setRestoreMaxButtonVisible(bool) {}

    void setCentralWidget(QWidget *widget);
    [[nodiscard]] auto shadowPadding() const -> int { return 0; }
};
#endif

} // namespace Widgets
