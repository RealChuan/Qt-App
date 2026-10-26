#include "pomodoroplugin.hpp"

#include "pomodoropage.hpp"

#include <api/danmaku/idanmaku.hpp>
#include <extensionsystem/plugincontext.hpp>
#include <extensionsystem/pluginmanager.hpp>
#include <shell/functioncommand.hpp>
#include <shell/host.hpp>
#include <shell/registry.hpp>

#include <QTimer>

#include <algorithm>

namespace Pomodoro {

namespace {

constexpr int kDefaultSeconds = 25 * 60;
constexpr int kTickIntervalMs = 1000;
constexpr int kStatusTimeoutMs = 8000;

} // namespace

class PomodoroPlugin::PomodoroPluginPrivate
{
public:
    void start(int seconds);
    void togglePause();
    void reset();

    PomodoroPage *page = nullptr;
    std::unique_ptr<Shell::FunctionCommand> command;
    QTimer timer;
    int total = kDefaultSeconds;
    int remaining = 0;
    bool paused = false;
};

void PomodoroPlugin::PomodoroPluginPrivate::start(int seconds)
{
    total = std::max(1, seconds);
    remaining = total;
    paused = false;

    timer.start(kTickIntervalMs);
    page->setRunning(true);
    page->setPaused(false);
    page->showRemaining(remaining, total);
}

void PomodoroPlugin::PomodoroPluginPrivate::togglePause()
{
    // 没在跑且不是暂停态：还没开始，忽略
    if (!timer.isActive() && !paused)
        return;

    paused = !paused;
    if (paused)
        timer.stop();
    else
        timer.start(kTickIntervalMs);
    page->setPaused(paused);
}

void PomodoroPlugin::PomodoroPluginPrivate::reset()
{
    timer.stop();
    paused = false;
    remaining = total;

    page->setRunning(false);
    page->setPaused(false);
    page->showRemaining(remaining, total);
}

PomodoroPlugin::PomodoroPlugin()
    : d_ptr(std::make_unique<PomodoroPluginPrivate>())
{}

PomodoroPlugin::~PomodoroPlugin() = default;

auto PomodoroPlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    d_ptr->page = new PomodoroPage;

    d_ptr->timer.setTimerType(Qt::CoarseTimer);
    connect(&d_ptr->timer, &QTimer::timeout, this, &PomodoroPlugin::onTick);
    connect(d_ptr->page, &PomodoroPage::startRequested, this, &PomodoroPlugin::onStartRequested);
    connect(d_ptr->page, &PomodoroPage::pauseToggled, this, &PomodoroPlugin::onPauseToggled);
    connect(d_ptr->page, &PomodoroPage::resetRequested, this, &PomodoroPlugin::onResetRequested);

    d_ptr->command = std::make_unique<Shell::FunctionCommand>(
        u"pomodoroplugin.start"_s,
        tr("Pomodoro: start 25 minutes"),
        std::bind_front(&PomodoroPlugin::startPomodoro, this));

    d_ptr->page->showRemaining(d_ptr->total, d_ptr->total);
    return Utils::ResultOk;
}

void PomodoroPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(d_ptr->page);
    registry->addCommand(d_ptr->command.get());
}

void PomodoroPlugin::startPomodoro()
{
    d_ptr->start(kDefaultSeconds);
}

void PomodoroPlugin::onStartRequested(int seconds)
{
    d_ptr->start(seconds);
}

void PomodoroPlugin::onPauseToggled()
{
    d_ptr->togglePause();
}

void PomodoroPlugin::onResetRequested()
{
    d_ptr->reset();
}

void PomodoroPlugin::onTick()
{
    if (--d_ptr->remaining > 0) {
        d_ptr->page->showRemaining(d_ptr->remaining, d_ptr->total);
        return;
    }

    d_ptr->timer.stop();
    d_ptr->page->setRunning(false);
    d_ptr->page->setPaused(false);
    d_ptr->page->showRemaining(0, d_ptr->total);
    notify(tr("Pomodoro finished. Take a break."));
}

void PomodoroPlugin::notify(const QString &text)
{
    // 先尝试服务（danmaku 插件在场时才有），没有就退回外壳的状态栏提示。
    // 注意：插件不要自己建托盘图标，否则关闭主窗口后会留下一个无法退出的图标。
    if (auto *danmaku = ExtensionSystem::PluginManager::object<Danmaku::IDanmaku>()) {
        danmaku->push(text, QColor(255, 150, 90));
        return;
    }

    if (Shell::host())
        Shell::host()->showStatusMessage(text, kStatusTimeoutMs);
}

} // namespace Pomodoro
