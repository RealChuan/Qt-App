#include "pomodoroplugin.hpp"

#include "pomodoropage.hpp"
#include "pomodoroworker.hpp"

#include <api/danmaku/idanmaku.hpp>
#include <extensionsystem/pluginmanager.hpp>
#include <shell/command.hpp>
#include <shell/host.hpp>
#include <shell/registry.hpp>

#include <QThread>

namespace Pomodoro {

class ToggleCommand final : public Shell::ICommand
{
public:
    explicit ToggleCommand(PomodoroPlugin *plugin)
        : m_plugin(plugin)
    {}

    auto id() const -> QString override { return QStringLiteral("pomodoroplugin.toggle"); }
    auto title() const -> QString override { return QStringLiteral("Pomodoro: start 25 minutes"); }
    auto subtitle() const -> QString override { return QStringLiteral("pomodoroplugin.page"); }
    void execute() override { m_plugin->toggle(); }

private:
    PomodoroPlugin *m_plugin;
};

PomodoroPlugin::PomodoroPlugin() = default;
PomodoroPlugin::~PomodoroPlugin() = default;

auto PomodoroPlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    m_page = new PomodoroPage;
    m_command = std::make_unique<ToggleCommand>(this);

    m_thread = new QThread(this);
    m_worker = new PomodoroWorker;
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_page, &PomodoroPage::startRequested, this, [this](int seconds) {
        m_total = seconds;
        startTimer(seconds);
    });
    connect(m_page, &PomodoroPage::pauseRequested, m_worker, &PomodoroWorker::pause);
    connect(m_page, &PomodoroPage::resumeRequested, m_worker, &PomodoroWorker::resume);
    connect(m_page, &PomodoroPage::resetRequested, m_worker, &PomodoroWorker::reset);
    connect(m_worker, &PomodoroWorker::tick, this, [this](int remaining) {
        m_page->setRemaining(remaining, m_total);
    });
    connect(m_worker, &PomodoroWorker::finished, this, [this] {
        m_page->setRunning(false);
        notify(tr("Pomodoro finished. Take a break."));
    });

    m_thread->start();
    return Utils::ResultOk;
}

void PomodoroPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(m_page);
    registry->addCommand(m_command.get());
}

auto PomodoroPlugin::aboutToShutdown() -> ExtensionSystem::IPlugin::ShutdownFlag
{
    if (m_thread) {
        m_thread->quit();
        m_thread->wait(3000);
        m_worker = nullptr; // deleteLater 在 worker 线程结束时执行
    }
    return ExtensionSystem::IPlugin::ShutdownFlag::Synchronous;
}

void PomodoroPlugin::toggle()
{
    m_total = 25 * 60;
    startTimer(m_total);
}

void PomodoroPlugin::startTimer(int seconds)
{
    m_page->setRunning(true);
    QMetaObject::invokeMethod(m_worker, [worker = m_worker, seconds] {
        worker->start(seconds);
    }, Qt::QueuedConnection);
}

void PomodoroPlugin::notify(const QString &text)
{
    // 先尝试服务（danmaku 插件在场时才有），没有就退回外壳的状态栏提示。
    // 注意：插件不要自己建托盘图标，否则关闭主窗口后会留下一个无法退出的图标。
    if (auto *danmaku = ExtensionSystem::PluginManager::getObject<Danmaku::IDanmaku>()) {
        danmaku->push(text, QColor(255, 150, 90));
        return;
    }

    if (Shell::host())
        Shell::host()->showStatusMessage(text, 8000);
}

} // namespace Pomodoro
