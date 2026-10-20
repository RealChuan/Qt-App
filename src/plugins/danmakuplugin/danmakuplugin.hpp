#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace Danmaku {

class DanmakuOverlay;
class DanmakuService;
class PushCommand;

// 服务提供者示例：把自己实现的服务放进对象池，任何插件都能取用。
class DanmakuPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "danmakuplugin.json")

public:
    DanmakuPlugin();
    ~DanmakuPlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;
    ExtensionSystem::IPlugin::ShutdownFlag aboutToShutdown() override;

    void pushDemoMessage();

private:
    DanmakuOverlay *m_overlay = nullptr;
    DanmakuService *m_service = nullptr;
    std::unique_ptr<PushCommand> m_command;
};

} // namespace Danmaku
