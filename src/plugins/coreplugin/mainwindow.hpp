#pragma once

#include <shell/host.hpp>
#include <shell/page.hpp>
#include <widgets/mainwidget.hpp>

#include <memory>

class QCloseEvent;

namespace Core {

class CommandPalette;

// 外壳主窗口：负责侧边栏导航、页面堆栈、命令面板与托盘。
// 页面来自 Shell::Registry，本类不直接认识任何具体插件。
class MainWindow final : public Widgets::MainWidget, public Shell::Host
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Shell::Host
    auto mainWindow() -> QWidget * override;
    void showPage(QAnyStringView pageId) override;
    void showStatusMessage(QAnyStringView text, int timeoutMs) override;
    auto askForRestart(QAnyStringView reason) -> bool override;

    void openCommandPalette();

    // 退出前调用：先摘掉托盘图标，窗口本身由调用方销毁。
    void prepareShutdown();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    class MainWindowPrivate;
    std::unique_ptr<MainWindowPrivate> d_ptr;
};

} // namespace Core
