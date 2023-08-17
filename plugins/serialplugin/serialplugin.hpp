#ifndef SERIALPLUGIN_HPP
#define SERIALPLUGIN_HPP

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

namespace Plugin {

class SerialPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit SerialPluginWidget(QObject *parent = nullptr);
};

class SerialPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "serialplugin.json")
public:
    SerialPlugin() = default;

    auto initialize(const QStringList &arguments, QString *errorString) -> bool override;
    void extensionsInitialized() override {}
};

} // namespace Plugin

#endif // SERIALPLUGIN_HPP
