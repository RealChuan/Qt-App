#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace Core {

class AboutPage;
class ConfigPage;
class MainWindow;
class PluginPage;

// 外壳插件：创建主窗口、把自身注册成 Shell::Host，并贡献设置/关于页面。
// 它是唯一被标记为 Required 的插件——没有它就没有界面。
class CorePlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "coreplugin.json")

public:
    CorePlugin();
    ~CorePlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;
    ExtensionSystem::IPlugin::ShutdownFlag aboutToShutdown() override;

private:
    std::unique_ptr<MainWindow> m_mainWindow;
    std::unique_ptr<ConfigPage> m_configPage;
    std::unique_ptr<PluginPage> m_pluginPage;
    std::unique_ptr<AboutPage> m_aboutPage;
};

} // namespace Core
