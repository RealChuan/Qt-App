#pragma once

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace Fractal {

class FractalWidget;
class ResetCommand;

class FractalPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "fractalplugin.json")

public:
    FractalPlugin();
    ~FractalPlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;

private:
    FractalWidget *m_widget = nullptr;
    std::unique_ptr<ResetCommand> m_command;
};

} // namespace Fractal
