#include "colorpickerplugin.hpp"

#include "colorpickerpage.hpp"
#include "colorpickerwidget.hpp"

#include <shell/functioncommand.hpp>
#include <shell/registry.hpp>
#include <utils/uihelpers.hpp>
#include <utils/utils.hpp>

namespace ColorPicker {

class ColorPickerPlugin::ColorPickerPluginPrivate
{
public:
    ColorPickerPage *page = nullptr;
    std::unique_ptr<ColorPickerWidget> picker;
    std::unique_ptr<Shell::FunctionCommand> command;
};

ColorPickerPlugin::ColorPickerPlugin()
    : d_ptr(std::make_unique<ColorPickerPluginPrivate>())
{}

ColorPickerPlugin::~ColorPickerPlugin() = default;

auto ColorPickerPlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    d_ptr->page = new ColorPickerPage;
    connect(d_ptr->page, &ColorPickerPage::openRequested, this, &ColorPickerPlugin::openPicker);

    d_ptr->command = std::make_unique<Shell::FunctionCommand>(
        QStringLiteral("colorpickerplugin.pick"),
        tr("Color picker: pick a color"),
        std::bind_front(&ColorPickerPlugin::openPicker, this));
    d_ptr->command->withSubtitle(tr("Magnifier follows the cursor, click to copy HEX"));

    return Utils::ResultOk;
}

void ColorPickerPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(d_ptr->page);
    registry->addCommand(d_ptr->command.get());
}

auto ColorPickerPlugin::aboutToShutdown() -> ExtensionSystem::IPlugin::ShutdownFlag
{
    // 工具窗不挂在主窗口下，退出前显式释放，避免它比插件活得更久。
    d_ptr->picker.reset();
    return ExtensionSystem::IPlugin::ShutdownFlag::Synchronous;
}

void ColorPickerPlugin::openPicker()
{
    if (!d_ptr->picker)
        d_ptr->picker = std::make_unique<ColorPickerWidget>();
    Utils::restoreAndActivate(d_ptr->picker.get());
}

} // namespace ColorPicker
