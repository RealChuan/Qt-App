#include "coreplugin.hpp"

#include "aboutpage.hpp"
#include "appearancepage.hpp"
#include "mainwindow.hpp"
#include "pluginpage.hpp"

#include <shell/functioncommand.hpp>
#include <shell/host.hpp>
#include <shell/registry.hpp>
#include <utils/uihelpers.hpp>
#include <utils/utils.hpp>

#include <QApplication>
#include <QPointer>

namespace Core {

namespace {

QPointer<MainWindow> s_mainWindow;

auto dialogParent() -> QWidget *
{
    QWidget *active = QApplication::activeModalWidget();
    if (!active)
        active = QApplication::activeWindow();
    if (!active || active->windowFlags().testFlag(Qt::Popup))
        active = s_mainWindow.data();
    return active;
}

} // namespace

class CorePlugin::CorePluginPrivate
{
public:
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<AppearancePage> appearancePage;
    std::unique_ptr<PluginPage> pluginPage;
    std::unique_ptr<AboutPage> aboutPage;
    std::unique_ptr<Shell::FunctionCommand> quitCommand;
};

CorePlugin::CorePlugin()
    : d_ptr(std::make_unique<CorePluginPrivate>())
{}

// 析构必须在成员类型完整可见的翻译单元里定义，否则 moc 生成的代码
// 无法实例化 unique_ptr 成员的删除器。
CorePlugin::~CorePlugin() = default;

auto CorePlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    d_ptr->mainWindow = std::make_unique<MainWindow>();
    s_mainWindow = d_ptr->mainWindow.get();
    Shell::setHost(d_ptr->mainWindow.get());
    Utils::setDialogParentGetter(dialogParent);
    return Utils::ResultOk;
}

void CorePlugin::extensionsInitialized()
{
    d_ptr->appearancePage = std::make_unique<AppearancePage>();
    d_ptr->pluginPage = std::make_unique<PluginPage>();
    d_ptr->aboutPage = std::make_unique<AboutPage>();

    // 外壳自己贡献一条"退出"命令，顺便作为命令面板的示例。
    d_ptr->quitCommand = std::make_unique<Shell::FunctionCommand>(
        QStringLiteral("coreplugin.quit"), tr("Application: quit"), &Utils::quitApplication);

    auto *registry = Shell::Registry::instance();
    registry->addPage(d_ptr->appearancePage.get());
    registry->addPage(d_ptr->pluginPage.get());
    registry->addPage(d_ptr->aboutPage.get());
    registry->addCommand(d_ptr->quitCommand.get());
}

auto CorePlugin::aboutToShutdown() -> ExtensionSystem::IPlugin::ShutdownFlag
{
    Shell::setHost(nullptr);
    s_mainWindow = nullptr;

    if (d_ptr->mainWindow) {
        // 关掉托盘图标并把窗口销毁放在 QApplication 还活着的时候完成。
        // 否则插件库要到进程退出、qlibraryCleanup() 阶段才卸载，那时再析构
        // QWidget/QMenu 已经没有 QApplication，会崩，托盘图标也会残留。
        d_ptr->mainWindow->prepareShutdown();

        // 先删外壳自己的页面，再删主窗口：页面 widget 会随 stack 一起销毁。
        d_ptr->appearancePage.reset();
        d_ptr->pluginPage.reset();
        d_ptr->aboutPage.reset();
        d_ptr->mainWindow.reset();
    }

    return ExtensionSystem::IPlugin::ShutdownFlag::Synchronous;
}

} // namespace Core
