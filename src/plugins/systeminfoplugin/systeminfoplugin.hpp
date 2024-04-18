#ifndef SYSTEMINFOPLUGIN_HPP
#define SYSTEMINFOPLUGIN_HPP

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

namespace Plugin {

class SystemInfoPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit SystemInfoPluginWidget(QObject *parent = nullptr);
};

class SystemInfoPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "systeminfoplugin.json")
public:
    SystemInfoPlugin() = default;

    auto initialize(const QStringList &arguments, QString *errorString) -> bool override;
    void extensionsInitialized() override {}
};

} // namespace Plugin

#endif // SYSTEMINFOPLUGIN_HPP
