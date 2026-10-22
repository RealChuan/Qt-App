#include "particlesplugin.hpp"

#include "particlespage.hpp"

#include <shell/functioncommand.hpp>
#include <shell/registry.hpp>

namespace Particles {

class ParticlesPlugin::ParticlesPluginPrivate
{
public:
    ParticlesPage *page = nullptr;
    std::unique_ptr<Shell::FunctionCommand> command;
};

ParticlesPlugin::ParticlesPlugin()
    : d_ptr(std::make_unique<ParticlesPluginPrivate>())
{}

ParticlesPlugin::~ParticlesPlugin() = default;

auto ParticlesPlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    d_ptr->page = new ParticlesPage;
    d_ptr->command = std::make_unique<Shell::FunctionCommand>(
        QStringLiteral("particlesplugin.burst"),
        tr("Particles: launch a burst"),
        std::bind_front(&ParticlesPlugin::burstInCenter, this));
    return Utils::ResultOk;
}

void ParticlesPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(d_ptr->page);
    registry->addCommand(d_ptr->command.get());
}

void ParticlesPlugin::burstInCenter()
{
    d_ptr->page->burst(QPointF(d_ptr->page->width() / 2.0, d_ptr->page->height() / 2.0));
}

} // namespace Particles
