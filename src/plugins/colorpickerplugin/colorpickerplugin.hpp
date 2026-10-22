#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace ColorPicker {

class ColorPickerPage;
class ColorPickerWidget;

// 演示"插件拥有自己的独立工具窗"：页面只提供一个入口，真正的窗口由插件按需创建。
class ColorPickerPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "colorpickerplugin.json")

public:
    ColorPickerPlugin();
    ~ColorPickerPlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;
    ExtensionSystem::IPlugin::ShutdownFlag aboutToShutdown() override;

    void openPicker();

private:
    class ColorPickerPluginPrivate;
    std::unique_ptr<ColorPickerPluginPrivate> d_ptr;
};

} // namespace ColorPicker
