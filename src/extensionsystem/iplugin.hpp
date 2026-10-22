#pragma once

#include "extensionsystem_global.hpp"

#include <utils/result.hpp>

#include <QObject>

namespace ExtensionSystem {

class PluginContext;
class PluginSpec;

// 插件 IID：每个插件在 Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" ...) 里写的字面量
// 必须与这里一致。moc 不展开宏，所以插件侧无法引用这个常量，只能靠注释互相提醒。
inline constexpr auto kPluginIID = "Youth.Qt.plugin";

class EXTENSIONSYSTEM_EXPORT IPlugin : public QObject
{
    Q_OBJECT

public:
    enum class ShutdownFlag { Synchronous, Asynchronous };

    explicit IPlugin(QObject *parent = nullptr);
    ~IPlugin() override;

    // 插件库加载成功后调用一次。返回错误则插件停在该状态，
    // 依赖它的插件不会被初始化（见 PluginManager::loadPlugins）。
    virtual Utils::Result<> initialize(const PluginContext &context) = 0;

    // 所有插件的 initialize() 成功之后调用。UI 贡献放这里：
    // 此时其他插件注册的服务都已经可用。
    virtual void extensionsInitialized() {}

    // 应用退出前调用，顺序与初始化相反。返回 Asynchronous 时，
    // 插件需要在清理完成后 emit asynchronousShutdownFinished()。
    virtual ShutdownFlag aboutToShutdown() { return ShutdownFlag::Synchronous; }

signals:
    void asynchronousShutdownFinished();
};

} // namespace ExtensionSystem
