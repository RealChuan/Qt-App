#pragma once

#include <crashpadclient/crashpad.hpp>
#include <widgets/dialog.hpp>

#include <QString>

#include <memory>
#include <vector>

namespace Crash {

// 上次运行异常退出时的启动期提示窗。
// 定制窗口，只有本应用用，所以放在 app 目录而不是 widgets 模块。
// 由 main() 在插件加载前弹出：插件初始化阶段崩溃时只有挡在启动流程里才能看到提示。
class CrashDialog final : public Widgets::Dialog
{
    Q_OBJECT

public:
    enum class Action {
        Dismiss, // 用户看过就关掉，报告标记为已处理
        Restart, // 退出当前进程并重启
    };

    CrashDialog(
        std::vector<Crashpad::ReportInfo> reports,
        QString crashPath,
        QString logPath,
        QWidget *parent = nullptr);
    ~CrashDialog() override;

    [[nodiscard]] auto action() const -> Action;

private slots:
    void onOpenFolders();
    void onRestartClicked();

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    void setupConnections();

    class CrashDialogPrivate;
    std::unique_ptr<CrashDialogPrivate> d_ptr;
};

} // namespace Crash
