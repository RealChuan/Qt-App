#include "danmakuplugin.hpp"

#include "danmakuoverlay.hpp"
#include "danmakuservice.hpp"

#include <extensionsystem/pluginmanager.hpp>
#include <shell/functioncommand.hpp>
#include <shell/host.hpp>
#include <shell/registry.hpp>

#include <QDateTime>

namespace Danmaku {

class DanmakuPlugin::DanmakuPluginPrivate
{
public:
    DanmakuOverlay *overlay = nullptr;
    DanmakuService *service = nullptr;
    std::unique_ptr<Shell::FunctionCommand> command;
};

DanmakuPlugin::DanmakuPlugin()
    : d_ptr(std::make_unique<DanmakuPluginPrivate>())
{}

DanmakuPlugin::~DanmakuPlugin() = default;

auto DanmakuPlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    d_ptr->service = new DanmakuService(this);
    ExtensionSystem::PluginManager::addObject(d_ptr->service);

    d_ptr->command = std::make_unique<Shell::FunctionCommand>(
        u"danmakuplugin.push"_s,
        tr("Danmaku: send a message"),
        std::bind_front(&DanmakuPlugin::pushDemoMessage, this));
    d_ptr->command->withSubtitle(tr("Broadcast to the overlay"));

    return Utils::ResultOk;
}

void DanmakuPlugin::extensionsInitialized()
{
    QWidget *window = Shell::host() ? Shell::host()->mainWindow() : nullptr;
    if (window) {
        d_ptr->overlay = new DanmakuOverlay(window);
        d_ptr->service->setOverlay(d_ptr->overlay);
    }

    Shell::Registry::instance()->addCommand(d_ptr->command.get());
}

auto DanmakuPlugin::aboutToShutdown() -> ExtensionSystem::IPlugin::ShutdownFlag
{
    ExtensionSystem::PluginManager::removeObject(d_ptr->service);
    d_ptr->service->setOverlay(nullptr);

    delete d_ptr->overlay;
    d_ptr->overlay = nullptr;
    return ExtensionSystem::IPlugin::ShutdownFlag::Synchronous;
}

void DanmakuPlugin::pushDemoMessage()
{
    d_ptr->service->push(
        tr("Hello from danmakuplugin at %1")
            .arg(QDateTime::currentDateTime().toString(u"HH:mm:ss"_s)),
        QColor(120, 220, 255));

    // 顺便演示"另一个插件也能推"：通过对象池把自己找回来。
    if (auto *service = ExtensionSystem::PluginManager::object<IDanmaku>()) {
        service->push(tr("%1 works").arg("PluginManager::object<IDanmaku>()"), QColor(255, 200, 120));
    }
}

} // namespace Danmaku
