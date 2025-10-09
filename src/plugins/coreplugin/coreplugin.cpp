#include "coreplugin.hpp"
#include "coreplugintr.h"
#include "icore.h"
#include "mainwindow.h"

#include <extensionsystem/iplugin.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/guiutils.h>
#include <utils/id.h>
#include <utils/utils.hpp>

#include <QApplication>
#include <QPointer>

namespace Plugin {

static QPointer<MainWindow> mainwindowPtr;

QWidget *dialogParent()
{
    QWidget *active = QApplication::activeModalWidget();
    if (!active)
        active = QApplication::activeWindow();
    if (!active || active->windowFlags().testFlag(Qt::SplashScreen)
        || active->windowFlags().testFlag(Qt::Popup)) {
        active = mainwindowPtr;
    }
    return active;
}

class CorePlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "coreplugin.json")
public:
    ~CorePlugin() override {}

    Utils::Result<> initialize(const QStringList &arguments) override
    {
        m_corePtr.reset(new ICore);
        m_mainWindowPtr.reset(new MainWindow);
        mainwindowPtr = m_mainWindowPtr.data();
        Utils::setDialogParentGetter(dialogParent);

        return Utils::ResultOk;
    }

    void extensionsInitialized() override { m_mainWindowPtr->extensionsInitialized(); }

    QObject *remoteCommand(const QStringList &, const QString &, const QStringList &) override
    {
        Utils::restoreAndActivate(mainwindowPtr.data());
        return nullptr;
    }

private:
    QScopedPointer<MainWindow> m_mainWindowPtr;
    QScopedPointer<ICore> m_corePtr;
};

} // namespace Plugin

#include "coreplugin.moc"
