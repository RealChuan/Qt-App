#include "coreplugin.hpp"

#include "aboutpage.hpp"
#include "configpage.hpp"
#include "mainwindow.hpp"
#include "pluginpage.hpp"

#include <shell/command.hpp>
#include <shell/host.hpp>
#include <shell/registry.hpp>
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

// 外壳自己贡献一条"退出"命令，顺便作为命令面板的示例。
class QuitCommand final : public Shell::ICommand
{
public:
    auto id() const -> QString override { return QStringLiteral("coreplugin.quit"); }
    auto title() const -> QString override { return QStringLiteral("Application: quit"); }
    void execute() override { Utils::quitApplication(); }
};

} // namespace

CorePlugin::CorePlugin() = default;

// 析构必须在成员类型完整可见的翻译单元里定义，否则 moc 生成的代码
// 无法实例化 unique_ptr 成员的删除器。
CorePlugin::~CorePlugin() = default;

auto CorePlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    m_mainWindow = std::make_unique<MainWindow>();
    s_mainWindow = m_mainWindow.get();
    Shell::setHost(m_mainWindow.get());
    Utils::setDialogParentGetter(dialogParent);
    return Utils::ResultOk;
}

void CorePlugin::extensionsInitialized()
{
    m_configPage = std::make_unique<ConfigPage>();
    m_pluginPage = std::make_unique<PluginPage>();
    m_aboutPage = std::make_unique<AboutPage>();

    auto *registry = Shell::Registry::instance();
    registry->addPage(m_configPage.get());
    registry->addPage(m_pluginPage.get());
    registry->addPage(m_aboutPage.get());

    static QuitCommand quitCommand;
    registry->addCommand(&quitCommand);
}

auto CorePlugin::aboutToShutdown() -> ExtensionSystem::IPlugin::ShutdownFlag
{
    Shell::setHost(nullptr);
    s_mainWindow = nullptr;

    if (m_mainWindow) {
        // 关掉托盘图标并把窗口销毁放在 QApplication 还活着的时候完成。
        // 否则插件库要到进程退出、qlibraryCleanup() 阶段才卸载，那时再析构
        // QWidget/QMenu 已经没有 QApplication，会崩，托盘图标也会残留。
        m_mainWindow->prepareShutdown();

        // 先删外壳自己的页面，再删主窗口：页面 widget 会随 stack 一起销毁。
        m_configPage.reset();
        m_pluginPage.reset();
        m_aboutPage.reset();
        m_mainWindow.reset();
    }

    return ExtensionSystem::IPlugin::ShutdownFlag::Synchronous;
}

} // namespace Core
