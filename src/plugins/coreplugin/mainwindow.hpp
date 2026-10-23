#pragma once

#include <shell/host.hpp>
#include <shell/page.hpp>
#include <widgets/appwindow.hpp>

#include <QSystemTrayIcon>

#include <memory>

class QCloseEvent;
class QEvent;

namespace Core {

class CommandPalette;

// 外壳主窗口：负责侧边栏导航、页面堆栈、命令面板与托盘。
// 页面来自 Shell::Registry，本类不直接认识任何具体插件。
class MainWindow final : public Widgets::AppWindow, public Shell::Host
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Shell::Host
    [[nodiscard]] auto mainWindow() -> QWidget * override;
    void showPage(QAnyStringView pageId) override;
    void showStatusMessage(QAnyStringView text, int timeoutMs) override;
    [[nodiscard]] auto askForRestart(QAnyStringView reason) -> bool override;

    void openCommandPalette();

    // 退出前调用：先摘掉托盘图标，窗口本身由调用方销毁。
    void prepareShutdown();

private slots:
    void onSidebarButtonClicked(int index);
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onPagesChanged();
    void onThemeActionTriggered();
    void onLanguageActionTriggered();
    void onThemeApplied(bool dark);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void updateThemeAction();
    void setupConnections();

    class MainWindowPrivate;
    std::unique_ptr<MainWindowPrivate> d_ptr;
};

} // namespace Core
