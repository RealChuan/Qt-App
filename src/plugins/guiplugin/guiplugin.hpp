#ifndef GUIPLUGIN_HPP
#define GUIPLUGIN_HPP

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

namespace Plugin {

class GuiPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit GuiPluginWidget(QObject *parent = nullptr);
};

class GuiPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "guiplugin.json")
public:
    GuiPlugin() = default;

    auto initialize(const QStringList &arguments, QString *errorString) -> bool override;
    void extensionsInitialized() override {}
};

} // namespace Plugin

#endif // GUIPLUGIN_HPP
