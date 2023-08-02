#ifndef TCPPLUGIN_HPP
#define TCPPLUGIN_HPP

#include <core/corewidget.hpp>
#include <extensionsystem/iplugin.h>

namespace Plugin {

class TcpPluginWidget : public Core::CoreWidget
{
    Q_OBJECT
public:
    explicit TcpPluginWidget(QObject *parent = nullptr);
};

class TcpPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "tcpplugin.json")
public:
    TcpPlugin() = default;

    bool initialize(const QStringList &arguments, QString *errorString) override;
    void extensionsInitialized() override {}
};

} // namespace Plugin

#endif // TCPPLUGIN_HPP
