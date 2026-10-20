#pragma once

#include "extensionsystem_global.h"

#include <utils/filepath.h>
#include <utils/result.h>

#include <QAnyStringView>
#include <QList>
#include <QObject>
#include <QStringList>

class QSettings;

namespace ExtensionSystem {

class IPlugin;
class PluginSpec;

// 插件运行时。整个进程只有一个实例（由 main() 创建），对外暴露静态门面，
// 因为插件之间互相访问时没有更自然的入口。
class EXTENSIONSYSTEM_EXPORT PluginManager : public QObject
{
    Q_OBJECT

public:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager() override;

    [[nodiscard]] static auto instance() -> PluginManager *;

    // 扫描 + 解析依赖 + 初始化。失败时返回第一条必需插件相关的错误，
    // 逐条错误用 allErrors() 取。
    static auto loadPlugins() -> Utils::Result<>;
    static void shutdown();

    [[nodiscard]] static auto plugins() -> const QList<PluginSpec *> &;
    [[nodiscard]] static auto specById(QAnyStringView id) -> PluginSpec *;
    [[nodiscard]] static auto allErrors() -> QStringList;
    [[nodiscard]] static auto hasError() -> bool;

    [[nodiscard]] static auto pluginPaths() -> Utils::FilePaths;
    static void setPluginPaths(const Utils::FilePaths &paths);

    [[nodiscard]] static auto settings() -> QSettings *;
    [[nodiscard]] static auto globalSettings() -> QSettings *;
    static void writeSettings();

    [[nodiscard]] static auto isInitializationDone() -> bool;
    [[nodiscard]] static auto isShuttingDown() -> bool;

    // 软依赖用的对象池：插件把自己实现的服务放进来，别的插件按接口取用。
    static void addObject(QObject *object);
    static void removeObject(QObject *object);
    [[nodiscard]] static auto allObjects() -> QObjectList;
    [[nodiscard]] static auto getObjectByName(QStringView name) -> QObject *;

    template<typename T>
    static auto getObject() -> T *
    {
        const QObjectList objects = allObjects();
        for (QObject *object : objects) {
            if (auto *result = qobject_cast<T *>(object))
                return result;
        }
        return nullptr;
    }

    template<typename T>
    static auto getObjects() -> QList<T *>
    {
        QList<T *> result;
        const QObjectList objects = allObjects();
        for (QObject *object : objects) {
            if (auto *candidate = qobject_cast<T *>(object))
                result.append(candidate);
        }
        return result;
    }

signals:
    void objectAdded(QObject *object);
    void aboutToRemoveObject(QObject *object);
    void pluginsChanged();

private:
    static PluginManager *s_instance;
};

} // namespace ExtensionSystem
