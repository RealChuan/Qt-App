#include "coreplugin.h"
#include "mainwindow.h"

#include <QApplication>

namespace Plugin {

CorePlugin::CorePlugin() {}

CorePlugin::~CorePlugin() {}

bool CorePlugin::initialize(const QStringList &, QString *)
{
    m_mainWindowPtr.reset(new MainWindow);
    return true;
}

void CorePlugin::extensionsInitialized()
{
    m_mainWindowPtr->extensionsInitialized();
}

QObject *CorePlugin::remoteCommand(const QStringList &, const QString &, const QStringList &)
{
    m_mainWindowPtr->setWindowState(m_mainWindowPtr->windowState() & ~Qt::WindowMinimized);
    m_mainWindowPtr->show();
    m_mainWindowPtr->raise();
    m_mainWindowPtr->activateWindow();
    return nullptr;
}

} // namespace Plugin
