#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace Particles {

class ParticlesPage;

class ParticlesPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "particlesplugin.json")

public:
    ParticlesPlugin();
    ~ParticlesPlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;

    void burstInCenter();

private:
    class ParticlesPluginPrivate;
    std::unique_ptr<ParticlesPluginPrivate> d_ptr;
};

} // namespace Particles
