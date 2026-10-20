#include "colorpickerplugin.hpp"

#include "colorpickerpage.hpp"
#include "colorpickerwidget.hpp"

#include <shell/command.hpp>
#include <shell/registry.hpp>

namespace ColorPicker {

class OpenCommand final : public Shell::ICommand
{
public:
    explicit OpenCommand(ColorPickerPlugin *plugin)
        : m_plugin(plugin)
    {}

    auto id() const -> QString override { return QStringLiteral("colorpickerplugin.pick"); }
    auto title() const -> QString override { return QStringLiteral("Color picker: pick a color"); }
    auto subtitle() const -> QString override
    {
        return QStringLiteral("Magnifier follows the cursor, click to copy HEX");
    }
    void execute() override { m_plugin->openPicker(); }

private:
    ColorPickerPlugin *m_plugin;
};

ColorPickerPlugin::ColorPickerPlugin() = default;
ColorPickerPlugin::~ColorPickerPlugin() = default;

auto ColorPickerPlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    m_page = new ColorPickerPage;
    m_command = std::make_unique<OpenCommand>(this);
    connect(m_page, &ColorPickerPage::openRequested, this, &ColorPickerPlugin::openPicker);
    return Utils::ResultOk;
}

void ColorPickerPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(m_page);
    registry->addCommand(m_command.get());
}

auto ColorPickerPlugin::aboutToShutdown() -> ExtensionSystem::IPlugin::ShutdownFlag
{
    delete m_picker;
    m_picker = nullptr;
    return ExtensionSystem::IPlugin::ShutdownFlag::Synchronous;
}

void ColorPickerPlugin::openPicker()
{
    if (!m_picker)
        m_picker = new ColorPickerWidget;
    m_picker->show();
    m_picker->raise();
    m_picker->activateWindow();
}

} // namespace ColorPicker
