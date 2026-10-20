#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

class QThread;

namespace Pomodoro {

class PomodoroPage;
class PomodoroWorker;
class ToggleCommand;

// 服务消费者示例：
//   - 计时跑在 QThread 的 worker 里（演示线程归属与关停）
//   - 到点优先用 Danmaku::IDanmaku 播报；接口缺席时退回托盘通知（optional 依赖）
class PomodoroPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "pomodoroplugin.json")

public:
    PomodoroPlugin();
    ~PomodoroPlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;
    ExtensionSystem::IPlugin::ShutdownFlag aboutToShutdown() override;

    void toggle();

private:
    void notify(const QString &text);
    void startTimer(int seconds);

    PomodoroPage *m_page = nullptr;
    PomodoroWorker *m_worker = nullptr;
    QThread *m_thread = nullptr;
    std::unique_ptr<ToggleCommand> m_command;
    int m_total = 25 * 60;
};

} // namespace Pomodoro
