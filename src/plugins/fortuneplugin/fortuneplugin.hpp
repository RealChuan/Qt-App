#pragma once

#include "fortuneengine.hpp"

#include <extensionsystem/iplugin.hpp>

#include <memory>

namespace Fortune {

class Fortunepage;

class FortunePlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "fortuneplugin.json")

public:
    FortunePlugin();
    ~FortunePlugin() override;

    Utils::Result<> initialize(const ExtensionSystem::PluginContext &context) override;
    void extensionsInitialized() override;

private slots:
    void onProfileSaved(const Profile &profile);
    void onProfileCleared();
    void onFortuneCasted(const HistoryEntry &entry);

private:
    void castFromCommand();

    class FortunePluginPrivate;
    std::unique_ptr<FortunePluginPrivate> d_ptr;
};

} // namespace Fortune
