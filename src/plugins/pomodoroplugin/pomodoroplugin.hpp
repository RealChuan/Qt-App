#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace Pomodoro {

class PomodoroPage;

// 服务消费者示例：
//   - 倒计时用插件自己持有的 QTimer（主线程，1 秒精度足够），不额外开线程
//   - 到点优先用 Danmaku::IDanmaku 播报；接口缺席时退回外壳状态栏（optional 依赖）
class PomodoroPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "pomodoroplugin.json")

public:
    PomodoroPlugin();
    ~PomodoroPlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;

    void startPomodoro();

private slots:
    void onTick();
    void onStartRequested(int seconds);
    void onPauseToggled();
    void onResetRequested();

private:
    void notify(const QString &text);

    class PomodoroPluginPrivate;
    std::unique_ptr<PomodoroPluginPrivate> d_ptr;
};

} // namespace Pomodoro
