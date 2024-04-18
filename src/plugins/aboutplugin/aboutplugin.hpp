#ifndef ABOUTPLUGIN_HPP
#define ABOUTPLUGIN_HPP

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

namespace Plugin {

class AboutPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit AboutPluginWidget(QObject *parent = nullptr);
};

class AboutPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "aboutplugin.json")
public:
    AboutPlugin() = default;

    auto initialize(const QStringList &arguments, QString *errorString) -> bool override;
    void extensionsInitialized() override {}
};

} // namespace Plugin

#endif // ABOUTPLUGIN_HPP
