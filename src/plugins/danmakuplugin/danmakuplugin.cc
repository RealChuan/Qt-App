#include "danmakuplugin.hpp"

#include "danmakuoverlay.hpp"
#include "danmakuservice.hpp"

#include <extensionsystem/pluginmanager.hpp>
#include <shell/command.hpp>
#include <shell/host.hpp>
#include <shell/registry.hpp>

#include <QDateTime>

namespace Danmaku {

class PushCommand final : public Shell::ICommand
{
public:
    explicit PushCommand(DanmakuPlugin *plugin)
        : m_plugin(plugin)
    {}

    auto id() const -> QString override { return QStringLiteral("danmakuplugin.push"); }
    auto title() const -> QString override { return QStringLiteral("Danmaku: send a message"); }
    auto subtitle() const -> QString override
    {
        return QStringLiteral("Broadcast to the overlay");
    }
    void execute() override { m_plugin->pushDemoMessage(); }

private:
    DanmakuPlugin *m_plugin;
};

DanmakuPlugin::DanmakuPlugin() = default;
DanmakuPlugin::~DanmakuPlugin() = default;

auto DanmakuPlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    m_service = new DanmakuService(this);
    m_command = std::make_unique<PushCommand>(this);
    ExtensionSystem::PluginManager::addObject(m_service);
    return Utils::ResultOk;
}

void DanmakuPlugin::extensionsInitialized()
{
    QWidget *window = Shell::host() ? Shell::host()->mainWindow() : nullptr;
    if (window) {
        m_overlay = new DanmakuOverlay(window);
        m_service->setOverlay(m_overlay);
    }

    auto *registry = Shell::Registry::instance();
    registry->addCommand(m_command.get());
}

auto DanmakuPlugin::aboutToShutdown() -> ExtensionSystem::IPlugin::ShutdownFlag
{
    ExtensionSystem::PluginManager::removeObject(m_service);
    m_service->setOverlay(nullptr);

    delete m_overlay;
    m_overlay = nullptr;
    return ExtensionSystem::IPlugin::ShutdownFlag::Synchronous;
}

void DanmakuPlugin::pushDemoMessage()
{
    m_service->push(QStringLiteral("Hello from danmakuplugin at %1")
                        .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"))),
                    QColor(120, 220, 255));

    // 顺便演示"另一个插件也能推"：通过对象池把自己找回来。
    if (auto *service = ExtensionSystem::PluginManager::getObject<IDanmaku>())
        service->push(QStringLiteral("PluginManager::getObject<IDanmaku>() works"), QColor(255, 200, 120));
}

} // namespace Danmaku
