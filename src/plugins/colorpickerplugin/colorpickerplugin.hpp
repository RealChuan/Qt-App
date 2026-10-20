#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace ColorPicker {

class ColorPickerPage;
class ColorPickerWidget;
class OpenCommand;

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
    ColorPickerPage *m_page = nullptr;
    ColorPickerWidget *m_picker = nullptr;
    std::unique_ptr<OpenCommand> m_command;
};

} // namespace ColorPicker
