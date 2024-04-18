// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "extensionsystem_global.h"

#include <QObject>

#include <functional>

namespace ExtensionSystem {

namespace Internal {
class IPluginPrivate;
}

using TestCreator = std::function<QObject *()>;

class EXTENSIONSYSTEM_EXPORT IPlugin : public QObject
{
    Q_OBJECT

public:
    enum ShutdownFlag { SynchronousShutdown, AsynchronousShutdown };

    IPlugin();
    ~IPlugin() override;

    virtual auto initialize(const QStringList &arguments, QString *errorString) -> bool;
    virtual void extensionsInitialized() {}
    virtual auto delayedInitialize() -> bool { return false; }
    virtual auto aboutToShutdown() -> ShutdownFlag { return SynchronousShutdown; }
    virtual auto remoteCommand(const QStringList & /* options */,
                               const QString & /* workingDirectory */,
                               const QStringList & /* arguments */) -> QObject *
    {
        return nullptr;
    }

    // Deprecated in 10.0, use addTest()
    [[nodiscard]] virtual auto createTestObjects() const -> QVector<QObject *>;

protected:
    virtual void initialize() {}

    template<typename Test, typename... Args>
    void addTest(Args &&...args)
    {
        addTestCreator([args...] { return new Test(args...); });
    }
    void addTestCreator(const TestCreator &creator);

signals:
    void asynchronousShutdownFinished();

protected:
    void addObject(QObject *obj);

private:
    Internal::IPluginPrivate *d;
};

} // namespace ExtensionSystem
