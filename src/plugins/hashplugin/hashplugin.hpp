#ifndef HASHPLUGIN_HPP
#define HASHPLUGIN_HPP

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

namespace Plugin {

class HashPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit HashPluginWidget(QObject *parent = nullptr);
};

class HashPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "hashplugin.json")
public:
    HashPlugin() = default;

    auto initialize(const QStringList &arguments, QString *errorString) -> bool override;
    void extensionsInitialized() override {}
};

} // namespace Plugin

#endif // HASHPLUGIN_HPP
