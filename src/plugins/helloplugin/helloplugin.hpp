#ifndef HELLOPLUGIN_HPP
#define HELLOPLUGIN_HPP

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

namespace Plugin {

class HelloPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit HelloPluginWidget(QObject *parent = nullptr);
};

class HelloPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "helloplugin.json")
public:
    HelloPlugin() = default;

    auto initialize(const QStringList &arguments, QString *errorString) -> bool override;
    void extensionsInitialized() override {}
};

} // namespace Plugin

#endif // HELLOPLUGIN_HPP
