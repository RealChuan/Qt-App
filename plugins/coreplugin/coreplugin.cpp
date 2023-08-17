#include "coreplugin.h"
#include "mainwindow.h"

#include <QApplication>

namespace Plugin {

CorePlugin::CorePlugin() = default;

CorePlugin::~CorePlugin() = default;

auto CorePlugin::initialize(const QStringList &, QString *) -> bool
{
    m_mainWindowPtr.reset(new MainWindow);
    return true;
}

void CorePlugin::extensionsInitialized()
{
    m_mainWindowPtr->extensionsInitialized();
}

auto CorePlugin::remoteCommand(const QStringList &, const QString &, const QStringList &)
    -> QObject *
{
    m_mainWindowPtr->setWindowState(m_mainWindowPtr->windowState() & ~Qt::WindowMinimized);
    m_mainWindowPtr->show();
    m_mainWindowPtr->raise();
    m_mainWindowPtr->activateWindow();
    return nullptr;
}

} // namespace Plugin
