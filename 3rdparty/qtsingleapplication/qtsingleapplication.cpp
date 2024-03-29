/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "qtsingleapplication.h"
#include "qtlocalpeer.h"

#include <qtlockedfile.h>

#include <QDir>
#include <QFileOpenEvent>
#include <QSharedMemory>
#include <QWidget>

namespace SharedTools {

static const int instancesSize = 1024;

static auto instancesLockFilename(const QString &appSessionId) -> QString
{
    const QChar slash(QLatin1Char('/'));
    QString res = QDir::tempPath();
    if (!res.endsWith(slash))
        res += slash;
    return res + appSessionId + QLatin1String("-instances");
}

QtSingleApplication::QtSingleApplication(const QString &appId, int &argc, char **argv)
    : QApplication(argc, argv),
      firstPeer(-1),
      pidPeer(0)
{
    this->appId = appId;

    const QString appSessionId = QtLocalPeer::appSessionId(appId);

    // This shared memory holds a zero-terminated array of active (or crashed) instances
    instances = new QSharedMemory(appSessionId, this);
    actWin = 0;
    block = false;

    // First instance creates the shared memory, later instances attach to it
    const bool created = instances->create(instancesSize);
    if (!created) {
        if (!instances->attach()) {
            qWarning() << "Failed to initialize instances shared memory: "
                       << instances->errorString();
            delete instances;
            instances = 0;
            return;
        }
    }

    // QtLockedFile is used to workaround QTBUG-10364
    QtLockedFile lockfile(instancesLockFilename(appSessionId));

    lockfile.open(QtLockedFile::ReadWrite);
    lockfile.lock(QtLockedFile::WriteLock);
    qint64 *pids = static_cast<qint64 *>(instances->data());
    if (!created) {
        // Find the first instance that it still running
        // The whole list needs to be iterated in order to append to it
        for (; *pids != 0; ++pids) {
            if (firstPeer == -1 && isRunning(*pids))
                firstPeer = *pids;
        }
    }
    // Add current pid to list and terminate it
    *pids++ = QCoreApplication::applicationPid();
    *pids = 0;
    pidPeer = new QtLocalPeer(this, appId + QLatin1Char('-') +
                              QString::number(QCoreApplication::applicationPid()));
    connect(pidPeer, &QtLocalPeer::messageReceived, this, &QtSingleApplication::messageReceived);
    pidPeer->isClient();
    lockfile.unlock();
}

QtSingleApplication::~QtSingleApplication()
{
    if (instances == nullptr)
        return;
    const qint64 appPid = QCoreApplication::applicationPid();
    QtLockedFile lockfile(instancesLockFilename(QtLocalPeer::appSessionId(appId)));
    lockfile.open(QtLockedFile::ReadWrite);
    lockfile.lock(QtLockedFile::WriteLock);
    // Rewrite array, removing current pid and previously crashed ones
    qint64 *pids = static_cast<qint64 *>(instances->data());
    qint64 *newpids = pids;
    for (; *pids != 0; ++pids) {
        if (*pids != appPid && isRunning(*pids))
            *newpids++ = *pids;
    }
    *newpids = 0;
    lockfile.unlock();
}

auto QtSingleApplication::event(QEvent *event) -> bool
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *foe = static_cast<QFileOpenEvent*>(event);
        emit fileOpenRequest(foe->file());
        return true;
    }
    return QApplication::event(event);
}

auto QtSingleApplication::isRunning(qint64 pid) -> bool
{
    if (pid == -1) {
        pid = firstPeer;
        if (pid == -1)
            return false;
    }

    QtLocalPeer peer(this, appId + QLatin1Char('-') + QString::number(pid, 10));
    return peer.isClient();
}

auto QtSingleApplication::sendMessage(const QString &message, int timeout, qint64 pid) -> bool
{
    if (pid == -1) {
        pid = firstPeer;
        if (pid == -1)
            return false;
    }

    QtLocalPeer peer(this, appId + QLatin1Char('-') + QString::number(pid, 10));
    return peer.sendMessage(message, timeout, block);
}

auto QtSingleApplication::applicationId() const -> QString
{
    return appId;
}

void QtSingleApplication::setBlock(bool value)
{
    block = value;
}

void QtSingleApplication::setActivationWindow(QWidget *aw, bool activateOnMessage)
{
    actWin = aw;
    if (pidPeer == nullptr)
        return;
    if (activateOnMessage)
        connect(pidPeer, &QtLocalPeer::messageReceived, this, &QtSingleApplication::activateWindow);
    else
        disconnect(pidPeer, &QtLocalPeer::messageReceived, this, &QtSingleApplication::activateWindow);
}


auto QtSingleApplication::activationWindow() const -> QWidget*
{
    return actWin;
}


void QtSingleApplication::activateWindow()
{
    if (actWin != nullptr) {
        actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
        actWin->raise();
        actWin->activateWindow();
    }
}

} // namespace SharedTools
