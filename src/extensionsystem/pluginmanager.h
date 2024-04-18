// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "extensionsystem_global.h"

#include <aggregation/aggregate.h>
#include <utils/qtcsettings.h>

#include <QObject>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

namespace ExtensionSystem {
class IPlugin;
class PluginSpec;

namespace Internal {
class PluginManagerPrivate;
}

class EXTENSIONSYSTEM_EXPORT PluginManager : public QObject
{
    Q_OBJECT

public:
    static auto instance() -> PluginManager *;

    PluginManager();
    ~PluginManager() override;

    // Object pool operations
    static void addObject(QObject *obj);
    static void removeObject(QObject *obj);
    static auto allObjects() -> QVector<QObject *>;
    static auto listLock() -> QReadWriteLock *;

    // This is useful for soft dependencies using pure interfaces.
    template<typename T>
    static auto getObject() -> T *
    {
        QReadLocker lock(listLock());
        const QVector<QObject *> all = allObjects();
        for (QObject *obj : all) {
            if (T *result = qobject_cast<T *>(obj))
                return result;
        }
        return nullptr;
    }
    template<typename T, typename Predicate>
    static auto getObject(Predicate predicate) -> T *
    {
        QReadLocker lock(listLock());
        const QVector<QObject *> all = allObjects();
        for (QObject *obj : all) {
            if (T *result = qobject_cast<T *>(obj))
                if (predicate(result))
                    return result;
        }
        return 0;
    }

    template<typename T>
    static auto getObjects() -> QVector<T *>
    {
        QReadLocker lock(listLock());
        QVector<T *> results;
        QVector<QObject *> all = allObjects();
        foreach (QObject *obj, all) {
            T *result = qobject_cast<T *>(obj);
            if (result)
                results += result;
        }
        return results;
    }

    template<typename T, typename Predicate>
    static auto getObjects(Predicate predicate) -> QVector<T *>
    {
        QReadLocker lock(listLock());
        QVector<T *> results;
        QVector<QObject *> all = allObjects();
        foreach (QObject *obj, all) {
            T *result = qobject_cast<T *>(obj);
            if (result && predicate(result))
                results += result;
        }
        return results;
    }

    static auto getObjectByName(const QString &name) -> QObject *;

    // Plugin operations
    static auto loadQueue() -> QVector<PluginSpec *>;
    static void loadPlugins();
    static auto pluginPaths() -> QStringList;
    static void setPluginPaths(const QStringList &paths);
    static auto pluginIID() -> QString;
    static void setPluginIID(const QString &iid);
    static auto plugins() -> const QVector<PluginSpec *>;
    static auto pluginCollections() -> QHash<QString, QVector<PluginSpec *>>;
    static auto hasError() -> bool;
    static auto allErrors() -> const QStringList;
    static auto pluginsRequiringPlugin(PluginSpec *spec) -> const QSet<PluginSpec *>;
    static auto pluginsRequiredByPlugin(PluginSpec *spec) -> const QSet<PluginSpec *>;
    static void checkForProblematicPlugins();
    static auto specForPlugin(IPlugin *plugin) -> PluginSpec *;

    // Settings
    static void setSettings(Utils::QtcSettings *settings);
    static auto settings() -> Utils::QtcSettings *;
    static void setGlobalSettings(Utils::QtcSettings *settings);
    static auto globalSettings() -> Utils::QtcSettings *;
    static void writeSettings();

    // command line arguments
    static auto arguments() -> QStringList;
    static auto argumentsForRestart() -> QStringList;
    static auto parseOptions(const QStringList &args,
                             const QMap<QString, bool> &appOptions,
                             QMap<QString, QString> *foundAppOptions,
                             QString *errorString) -> bool;
    static void formatOptions(QTextStream &str, int optionIndentation, int descriptionIndentation);
    static void formatPluginOptions(QTextStream &str,
                                    int optionIndentation,
                                    int descriptionIndentation);
    static void formatPluginVersions(QTextStream &str);

    static auto serializedArguments() -> QString;

    static auto testRunRequested() -> bool;

#ifdef WITH_TESTS
    static bool registerScenario(const QString &scenarioId, std::function<bool()> scenarioStarter);
    static bool isScenarioRequested();
    static bool runScenario();
    static bool isScenarioRunning(const QString &scenarioId);
    // static void triggerScenarioPoint(const QVariant pointData); // ?? called from scenario point
    static bool finishScenario();
    static void waitForScenarioFullyInitialized();
    // signals:
    // void scenarioPointTriggered(const QVariant pointData); // ?? e.g. in StringTable::GC() -> post a call to quit into main thread and sleep for 5 seconds in the GC thread
#endif

    struct ProcessData
    {
        QString m_executable;
        QStringList m_args;
        QString m_workingPath;
        QString m_settingsPath;
    };

    static void setCreatorProcessData(const ProcessData &data);
    static auto creatorProcessData() -> ProcessData;

    static void profilingReport(const char *what, const PluginSpec *spec = nullptr);

    static auto platformName() -> QString;

    static auto isInitializationDone() -> bool;

    static void remoteArguments(const QString &serializedArguments, QObject *socket);
    static void shutdown();

    //static QString systemInformation();

signals:
    void objectAdded(QObject *obj);
    void aboutToRemoveObject(QObject *obj);

    void pluginsChanged();
    void initializationDone();
    void testsFinished(int failedTests);
    void scenarioFinished(int exitCode);

    friend class Internal::PluginManagerPrivate;
};

} // namespace ExtensionSystem
