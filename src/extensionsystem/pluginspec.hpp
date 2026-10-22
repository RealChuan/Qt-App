#pragma once

#include "extensionsystem_global.hpp"

#include <utils/filepath.hpp>
#include <utils/result.hpp>

#include <QList>
#include <QString>

#include <memory>

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

namespace ExtensionSystem {

class IPlugin;
class PluginManager;

namespace Internal {
class PluginManagerPrivate;
}

// 插件元数据里一条依赖声明：{"Id": "...", "Version": "...", "Type": "optional"}。
class EXTENSIONSYSTEM_EXPORT PluginDependency
{
public:
    enum class Type { Required, Optional };

    QString id;
    QString version;
    Type type = Type::Required;

    bool operator==(const PluginDependency &other) const = default;
};

// 一个插件的描述信息 + 运行状态。由 PluginManager 创建和驱动，插件自己不持有它。
class EXTENSIONSYSTEM_EXPORT PluginSpec
{
public:
    enum class State { Invalid, Read, Resolved, Loaded, Initialized, Running, Stopped };

    PluginSpec();
    ~PluginSpec();
    Q_DISABLE_COPY_MOVE(PluginSpec)

    // ---- 元数据（*.json 里读出来的字段）----
    [[nodiscard]] auto id() const -> QString;
    [[nodiscard]] auto name() const -> QString;
    [[nodiscard]] auto displayName() const -> QString;
    [[nodiscard]] auto version() const -> QString;
    [[nodiscard]] auto compatVersion() const -> QString;
    [[nodiscard]] auto vendor() const -> QString;
    [[nodiscard]] auto copyright() const -> QString;
    [[nodiscard]] auto license() const -> QString;
    [[nodiscard]] auto category() const -> QString;
    [[nodiscard]] auto description() const -> QString;
    [[nodiscard]] auto longDescription() const -> QString;
    [[nodiscard]] auto url() const -> QString;

    // ---- 状态 ----
    [[nodiscard]] auto state() const -> State;
    [[nodiscard]] auto stateText() const -> QString;
    [[nodiscard]] auto hasError() const -> bool;
    [[nodiscard]] auto errorString() const -> QString;

    // ---- 启用状态 ----
    [[nodiscard]] auto isRequired() const -> bool;
    [[nodiscard]] auto isEnabledBySettings() const -> bool;
    [[nodiscard]] auto isEffectivelyEnabled() const -> bool;
    void setEnabledBySettings(bool value);

    // ---- 依赖 ----
    [[nodiscard]] auto dependencies() const -> QList<PluginDependency>;
    [[nodiscard]] auto dependencySpecs() const -> QList<PluginSpec *>;

    // ---- 运行时 ----
    [[nodiscard]] auto filePath() const -> Utils::FilePath;
    [[nodiscard]] auto plugin() const -> IPlugin *;
    [[nodiscard]] auto loadTimeMs() const -> qint64;

    // "1.2.10" 与 "1.2.9" 比较；返回 <0 / 0 / >0。
    static auto versionCompare(const QString &lhs, const QString &rhs) -> int;

private:
    friend class PluginManager;
    friend class Internal::PluginManagerPrivate;

    auto readMetaData(const QJsonObject &metaData) -> Utils::Result<>;
    void setState(State state);
    void setError(QString error);
    void setPlugin(IPlugin *plugin);
    void setFilePath(Utils::FilePath path);
    void setLoadTimeMs(qint64 ms);
    void setDependencySpecs(QList<PluginSpec *> specs);

    struct Private;
    std::unique_ptr<Private> d;
};

} // namespace ExtensionSystem
