#include "particlesplugin.hpp"

#include "particleswidget.hpp"

#include <shell/command.hpp>
#include <shell/registry.hpp>

namespace Particles {

class BurstCommand final : public Shell::ICommand
{
public:
    explicit BurstCommand(ParticlesPlugin *plugin)
        : m_plugin(plugin)
    {}

    auto id() const -> QString override { return QStringLiteral("particlesplugin.burst"); }
    auto title() const -> QString override { return QStringLiteral("Particles: launch a burst"); }
    auto subtitle() const -> QString override { return QStringLiteral("particlesplugin.page"); }
    void execute() override { m_plugin->burstInCenter(); }

private:
    ParticlesPlugin *m_plugin;
};

ParticlesPlugin::ParticlesPlugin() = default;
ParticlesPlugin::~ParticlesPlugin() = default;

auto ParticlesPlugin::initialize(const ExtensionSystem::PluginContext &) -> Utils::Result<>
{
    m_widget = new ParticlesWidget;
    m_command = std::make_unique<BurstCommand>(this);
    return Utils::ResultOk;
}

void ParticlesPlugin::extensionsInitialized()
{
    auto *registry = Shell::Registry::instance();
    registry->addPage(m_widget);
    registry->addCommand(m_command.get());
}

void ParticlesPlugin::burstInCenter()
{
    m_widget->burst(QPointF(m_widget->width() / 2.0, m_widget->height() / 2.0));
}

} // namespace Particles
