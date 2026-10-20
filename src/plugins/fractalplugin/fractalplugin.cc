#include "fractalplugin.hpp"

#include "fractalwidget.hpp"

#include <shell/command.hpp>
#include <shell/registry.hpp>

namespace Fractal {

class ResetCommand final : public Shell::ICommand
{
public:
    explicit ResetCommand(FractalWidget *widget)
        : m_widget(widget)
    {}

    auto id() const -> QString override { return QStringLiteral("fractalplugin.reset"); }
    auto title() const -> QString override { return QStringLiteral("Fractal: reset view"); }
    auto subtitle() const -> QString override { return QStringLiteral("fractalplugin.page"); }
    void execute() override { m_widget->resetView(); }

private:
    FractalWidget *m_widget;
};

FractalPlugin::FractalPlugin() = default;
FractalPlugin::~FractalPlugin() = default;

auto FractalPlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    m_widget = new FractalWidget;
    m_command = std::make_unique<ResetCommand>(m_widget);
    return Utils::ResultOk;
}

void FractalPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(m_widget);
    registry->addCommand(m_command.get());
}

} // namespace Fractal
