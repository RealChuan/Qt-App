#ifndef COREPLUGIN_H
#define COREPLUGIN_H

#include <extensionsystem/iplugin.h>

namespace Plugin {

class MainWindow;
class CorePlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "coreplugin.json")
public:
    CorePlugin();
    ~CorePlugin() override;

    auto initialize(const QStringList &arguments, QString *errorString) -> bool override;
    void extensionsInitialized() override;
    auto remoteCommand(const QStringList &, const QString &, const QStringList &)
        -> QObject * override;

private:
    QScopedPointer<MainWindow> m_mainWindowPtr;
};

} // namespace Plugin

#endif // COREPLUGIN_H
