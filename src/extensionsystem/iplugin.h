// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "extensionsystem_global.h"

#include <utils/result.h>

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

    virtual Utils::Result<> initialize(const QStringList &arguments);
    virtual void extensionsInitialized() {}
    virtual bool delayedInitialize() { return false; }
    virtual ShutdownFlag aboutToShutdown() { return SynchronousShutdown; }
    virtual QObject *remoteCommand(const QStringList & /* options */,
                                   const QString & /* workingDirectory */,
                                   const QStringList & /* arguments */)
    {
        return nullptr;
    }

    template<typename Test, typename... Args>
    void addTest(Args &&...args)
    {
        addTestCreator([args...] { return new Test(args...); });
    }
    void addTestCreator(const TestCreator &creator);

protected:
    virtual void initialize() {}

signals:
    void asynchronousShutdownFinished();

protected:
    void addObject(QObject *obj);

private:
    Internal::IPluginPrivate *d = nullptr; // For potential extension. Currently unused.
};

} // namespace ExtensionSystem
