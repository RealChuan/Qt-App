#include "fractalplugin.hpp"

#include "fractalpage.hpp"

#include <shell/functioncommand.hpp>
#include <shell/registry.hpp>

namespace Fractal {

class FractalPlugin::FractalPluginPrivate
{
public:
    FractalPage *page = nullptr;
    std::unique_ptr<Shell::FunctionCommand> command;
};

FractalPlugin::FractalPlugin()
    : d_ptr(std::make_unique<FractalPluginPrivate>())
{}

FractalPlugin::~FractalPlugin() = default;

auto FractalPlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    d_ptr->page = new FractalPage;
    d_ptr->command = std::make_unique<Shell::FunctionCommand>(
        u"fractalplugin.reset"_s,
        tr("Fractal: reset view"),
        std::bind_front(&FractalPlugin::resetView, this));
    return Utils::ResultOk;
}

void FractalPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(d_ptr->page);
    registry->addCommand(d_ptr->command.get());
}

void FractalPlugin::resetView()
{
    d_ptr->page->resetView();
}

} // namespace Fractal
