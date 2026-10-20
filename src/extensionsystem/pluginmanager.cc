#include "pluginmanager.hpp"

#include "extensionsystemtr.h"
#include "iplugin.hpp"
#include "plugincontext.hpp"
#include "pluginspec.hpp"

#include <utils/appinfo.h>
#include <utils/settings.h>

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QJsonObject>
#include <QLibrary>
#include <QLoggingCategory>
#include <QPluginLoader>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QSettings>
#include <QSet>
#include <QTimer>
#include <QWriteLocker>

#include <algorithm>
#include <memory>
#include <vector>

Q_LOGGING_CATEGORY(lcPluginSystem, "qtapp.plugin", QtInfoMsg)

namespace ExtensionSystem {

namespace {

const char kIgnoredPluginsKey[] = "Plugins/Ignored";
const char kForceEnabledPluginsKey[] = "Plugins/ForceEnabled";

// 一次进程生命周期内的全部可变状态。集中放一个结构里，便于阅读与调试时整体观察。
struct ManagerState
{
    Utils::FilePaths paths;
    std::vector<std::unique_ptr<PluginSpec>> storage; // 拥有 PluginSpec
    QList<PluginSpec *> plugins;                      // 扫描顺序
    QList<PluginSpec *> loadOrder;                    // 依赖排序后的初始化顺序
    QList<QSettings *> pluginSettings;
    bool loaded = false;
    bool shuttingDown = false;
};

auto state() -> ManagerState &
{
    static ManagerState managerState;
    return managerState;
}

auto objectList() -> QObjectList &
{
    static QObjectList objects;
    return objects;
}

auto objectLock() -> QReadWriteLock &
{
    static QReadWriteLock lock;
    return lock;
}

} // namespace

namespace Internal {

// 加载器内部实现。声明为 PluginSpec 的友元，这样才能推进它的状态。
class PluginManagerPrivate
{
public:
    static void applySettings();
    static void scanPlugins();
    static void resolveDependencies();
    static auto buildLoadOrder() -> QList<PluginSpec *>;
    static auto findById(QAnyStringView id) -> PluginSpec *;
    static auto pluginSettingsFor(const PluginSpec *spec) -> QSettings *;

private:
    static auto visit(PluginSpec *spec, QSet<PluginSpec *> &visiting, QSet<PluginSpec *> &done,
                      QList<PluginSpec *> &order) -> bool;
};

void PluginManagerPrivate::applySettings()
{
    QStringList ignored;
    for (const QString &id :
         PluginManager::settings()->value(QLatin1StringView(kIgnoredPluginsKey)).toStringList()) {
        ignored.append(id.toLower());
    }

    for (PluginSpec *spec : std::as_const(state().plugins))
        spec->setEnabledBySettings(!ignored.contains(spec->id().toLower()));
}

auto PluginManagerPrivate::findById(QAnyStringView id) -> PluginSpec *
{
    const QString wanted = id.toString();
    for (PluginSpec *spec : std::as_const(state().plugins)) {
        if (spec->id().compare(wanted, Qt::CaseInsensitive) == 0)
            return spec;
    }
    return nullptr;
}

void PluginManagerPrivate::scanPlugins()
{
    auto &manager = state();
    for (const Utils::FilePath &path : std::as_const(manager.paths)) {
        const QDir directory(path.path());
        if (!directory.exists()) {
            qCWarning(lcPluginSystem) << "plugin path does not exist:" << path.toUserOutput();
            continue;
        }

        const QFileInfoList entries = directory.entryInfoList(QDir::Files);
        for (const QFileInfo &entry : entries) {
            const QString filePath = entry.absoluteFilePath();
            if (!QLibrary::isLibrary(filePath))
                continue;

            QPluginLoader probe(filePath);
            const QJsonObject metaData = probe.metaData();
            if (metaData.isEmpty())
                continue;
            if (metaData.value(QLatin1StringView("IID")).toString()
                != QString::fromLatin1(kPluginIID)) {
                qCWarning(lcPluginSystem) << "skip" << entry.fileName() << "(IID mismatch)";
                continue;
            }

            auto spec = std::make_unique<PluginSpec>();
            spec->setFilePath(Utils::FilePath::fromString(filePath));
            const auto result
                = spec->readMetaData(metaData.value(QLatin1StringView("MetaData")).toObject());
            if (!result) {
                qCWarning(lcPluginSystem)
                    << "invalid metadata in" << entry.fileName() << ':' << result.error();
                continue;
            }

            if (PluginSpec *existing = findById(spec->id())) {
                const QString error = Tr::tr("Duplicate plugin id \"%1\".").arg(spec->id());
                qCWarning(lcPluginSystem) << error << entry.fileName()
                                          << existing->filePath().toUserOutput();
                existing->setError(error);
                continue;
            }

            manager.plugins.append(spec.get());
            manager.storage.push_back(std::move(spec));
        }
    }
}

void PluginManagerPrivate::resolveDependencies()
{
    for (PluginSpec *spec : std::as_const(state().plugins)) {
        if (spec->hasError())
            continue;

        QList<PluginSpec *> resolved;
        bool missingRequired = false;
        for (const PluginDependency &dependency : spec->dependencies()) {
            PluginSpec *provider = findById(dependency.id);
            if (provider && !dependency.version.isEmpty()
                && PluginSpec::versionCompare(provider->version(), dependency.version) < 0) {
                provider = nullptr;
            }

            if (!provider) {
                if (dependency.type == PluginDependency::Type::Required) {
                    spec->setError(
                        Tr::tr("Required dependency \"%1\" (version %2 or newer) was not found.")
                            .arg(dependency.id, dependency.version));
                    missingRequired = true;
                    break;
                }
                qCWarning(lcPluginSystem)
                    << "optional dependency missing:" << dependency.id << "for" << spec->id();
                continue;
            }

            // 只有 required 依赖参与"启用/排序"判定：optional 依赖的提供方
            // 即便被禁用，也不应该把消费方一起拖下水（消费方自己做运行时降级）。
            if (dependency.type == PluginDependency::Type::Required)
                resolved.append(provider);
        }

        if (missingRequired)
            continue;
        spec->setDependencySpecs(resolved);
        spec->setState(PluginSpec::State::Resolved);
    }
}

auto PluginManagerPrivate::visit(PluginSpec *spec, QSet<PluginSpec *> &visiting,
                                 QSet<PluginSpec *> &done, QList<PluginSpec *> &order) -> bool
{
    if (done.contains(spec))
        return true;
    if (visiting.contains(spec)) {
        spec->setError(Tr::tr("Circular dependency detected at \"%1\".").arg(spec->id()));
        return false;
    }
    if (spec->hasError())
        return false;

    visiting.insert(spec);
    for (PluginSpec *dependency : spec->dependencySpecs()) {
        if (!dependency->isEffectivelyEnabled())
            continue;
        if (!visit(dependency, visiting, done, order)) {
            spec->setError(Tr::tr("Dependency \"%1\" failed: %2")
                               .arg(dependency->id(), dependency->errorString()));
            visiting.remove(spec);
            return false;
        }
    }
    visiting.remove(spec);
    done.insert(spec);
    order.append(spec);
    return true;
}

auto PluginManagerPrivate::buildLoadOrder() -> QList<PluginSpec *>
{
    QList<PluginSpec *> order;
    QSet<PluginSpec *> visiting;
    QSet<PluginSpec *> done;
    for (PluginSpec *spec : std::as_const(state().plugins)) {
        if (!spec->isEffectivelyEnabled())
            continue;
        visit(spec, visiting, done, order);
    }
    return order;
}

auto PluginManagerPrivate::pluginSettingsFor(const PluginSpec *spec) -> QSettings *
{
    QSettings *base = PluginManager::settings();
    auto *settings = new QSettings(base->fileName(), base->format());
    settings->beginGroup(QStringLiteral("PluginData/%1").arg(spec->id()));
    state().pluginSettings.append(settings);
    return settings;
}

} // namespace Internal

PluginManager *PluginManager::s_instance = nullptr;

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
    if (!s_instance)
        s_instance = this;
}

PluginManager::~PluginManager()
{
    if (s_instance == this)
        s_instance = nullptr;
}

auto PluginManager::instance() -> PluginManager *
{
    return s_instance;
}

auto PluginManager::loadPlugins() -> Utils::Result<>
{
    using Internal::PluginManagerPrivate;

    auto &manager = state();
    if (manager.loaded)
        return Utils::ResultOk;
    manager.loaded = true;

    PluginManagerPrivate::scanPlugins();
    if (manager.plugins.isEmpty()) {
        QStringList searched;
        for (const Utils::FilePath &path : std::as_const(manager.paths))
            searched.append(path.toUserOutput());
        return Utils::ResultError(
            Tr::tr("No plugins found in: %1").arg(searched.join(QStringLiteral(", "))));
    }

    PluginManagerPrivate::applySettings();
    PluginManagerPrivate::resolveDependencies();
    manager.loadOrder = PluginManagerPrivate::buildLoadOrder();

    QSet<QString> failed;
    for (PluginSpec *spec : std::as_const(manager.loadOrder)) {
        if (!spec->isEffectivelyEnabled() || spec->hasError()) {
            failed.insert(spec->id());
            continue;
        }

        const bool dependencyFailed = std::ranges::any_of(
            spec->dependencySpecs(),
            [&failed](const PluginSpec *dependency) { return failed.contains(dependency->id()); });
        if (dependencyFailed) {
            spec->setError(Tr::tr("A required dependency failed to load."));
            failed.insert(spec->id());
            continue;
        }

        QElapsedTimer timer;
        timer.start();

        auto *loader = new QPluginLoader(spec->filePath().path(), instance());
        // Qt 不建议卸载插件：销毁实例会让插件持有的静态状态与跨插件指针失效。
        loader->setLoadHints(QLibrary::PreventUnloadHint);

        auto *plugin = qobject_cast<IPlugin *>(loader->instance());
        if (!plugin) {
            spec->setError(loader->errorString().isEmpty() ? Tr::tr("File is not a valid plugin.")
                                                           : loader->errorString());
            failed.insert(spec->id());
            continue;
        }

        spec->setPlugin(plugin);
        spec->setState(PluginSpec::State::Loaded);

        QSettings *settings = PluginManagerPrivate::pluginSettingsFor(spec);
        const PluginContext context(spec, settings, Utils::appInfo().resources);
        const auto result = plugin->initialize(context);
        if (!result) {
            spec->setError(result.error());
            failed.insert(spec->id());
            continue;
        }

        spec->setLoadTimeMs(timer.elapsed());
        spec->setState(PluginSpec::State::Initialized);
        qCInfo(lcPluginSystem).noquote() << QStringLiteral("plugin initialized: %1 (%2 ms)")
                                                .arg(spec->id())
                                                .arg(spec->loadTimeMs());
    }

    for (PluginSpec *spec : std::as_const(manager.loadOrder)) {
        if (spec->state() != PluginSpec::State::Initialized)
            continue;
        if (spec->plugin())
            spec->plugin()->extensionsInitialized();
        spec->setState(PluginSpec::State::Running);
    }

    if (instance())
        emit instance()->pluginsChanged();

    for (PluginSpec *spec : std::as_const(manager.plugins)) {
        if (spec->hasError())
            qCWarning(lcPluginSystem).noquote()
                << QStringLiteral("plugin error: %1: %2").arg(spec->id(), spec->errorString());
        if (spec->isRequired() && spec->hasError())
            return Utils::ResultError(
                QStringLiteral("%1: %2").arg(spec->id(), spec->errorString()));
    }

    return Utils::ResultOk;
}

void PluginManager::shutdown()
{
    auto &manager = state();
    if (manager.shuttingDown)
        return;
    manager.shuttingDown = true;

    for (auto it = manager.loadOrder.crbegin(); it != manager.loadOrder.crend(); ++it) {
        PluginSpec *spec = *it;
        IPlugin *plugin = spec->plugin();
        if (!plugin || spec->state() != PluginSpec::State::Running)
            continue;

        if (plugin->aboutToShutdown() == IPlugin::ShutdownFlag::Asynchronous) {
            QEventLoop loop;
            QObject::connect(plugin,
                             &IPlugin::asynchronousShutdownFinished,
                             &loop,
                             &QEventLoop::quit);
            QTimer::singleShot(5000, &loop, &QEventLoop::quit);
            loop.exec();
        }
        spec->setState(PluginSpec::State::Stopped);
    }

    for (QSettings *settings : std::as_const(manager.pluginSettings))
        settings->sync();

    writeSettings();
}

auto PluginManager::plugins() -> const QList<PluginSpec *> &
{
    return state().plugins;
}

auto PluginManager::specById(QAnyStringView id) -> PluginSpec *
{
    return Internal::PluginManagerPrivate::findById(id);
}

auto PluginManager::allErrors() -> QStringList
{
    QStringList errors;
    for (const PluginSpec *spec : std::as_const(state().plugins)) {
        if (spec->hasError())
            errors.append(QStringLiteral("%1: %2").arg(spec->id(), spec->errorString()));
    }
    return errors;
}

auto PluginManager::hasError() -> bool
{
    for (const PluginSpec *spec : std::as_const(state().plugins)) {
        if (spec->hasError())
            return true;
    }
    return false;
}

auto PluginManager::pluginPaths() -> Utils::FilePaths
{
    return state().paths;
}

void PluginManager::setPluginPaths(const Utils::FilePaths &paths)
{
    state().paths = paths;
}

auto PluginManager::settings() -> QSettings *
{
    return &Utils::userSettings();
}

auto PluginManager::globalSettings() -> QSettings *
{
    return &Utils::installSettings();
}

void PluginManager::writeSettings()
{
    QStringList ignored;
    for (const PluginSpec *spec : std::as_const(state().plugins)) {
        if (!spec->isRequired() && !spec->isEnabledBySettings())
            ignored.append(spec->id());
    }
    Utils::setValueWithDefault(*settings(), QLatin1StringView(kIgnoredPluginsKey), ignored);
    Utils::setValueWithDefault(*settings(), QLatin1StringView(kForceEnabledPluginsKey), QStringList{});
}

auto PluginManager::isInitializationDone() -> bool
{
    return state().loaded;
}

auto PluginManager::isShuttingDown() -> bool
{
    return state().shuttingDown;
}

void PluginManager::addObject(QObject *object)
{
    if (!object)
        return;
    {
        QWriteLocker locker(&objectLock());
        if (objectList().contains(object))
            return;
        objectList().append(object);
    }
    if (instance())
        emit instance()->objectAdded(object);
}

void PluginManager::removeObject(QObject *object)
{
    if (!object)
        return;
    {
        QWriteLocker locker(&objectLock());
        if (!objectList().removeOne(object))
            return;
    }
    if (instance())
        emit instance()->aboutToRemoveObject(object);
}

auto PluginManager::allObjects() -> QObjectList
{
    QReadLocker locker(&objectLock());
    return objectList();
}

auto PluginManager::getObjectByName(QStringView name) -> QObject *
{
    const QObjectList objects = allObjects();
    for (QObject *object : objects) {
        if (object->objectName() == name)
            return object;
    }
    return nullptr;
}

} // namespace ExtensionSystem
