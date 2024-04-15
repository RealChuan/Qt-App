// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "extensionsystem_global.h"

#include <QHash>
#include <QStaticPlugin>
#include <QString>
#include <QVector>

QT_BEGIN_NAMESPACE
class QRegularExpression;
QT_END_NAMESPACE

namespace ExtensionSystem {

namespace Internal {

class OptionsParser;
class PluginSpecPrivate;
class PluginManagerPrivate;

} // Internal

class IPlugin;
class PluginView;

struct EXTENSIONSYSTEM_EXPORT PluginDependency
{
    enum Type {
        Required,
        Optional,
        Test
    };

    PluginDependency() : type(Required) {}

    friend size_t qHash(const PluginDependency &value);

    QString name;
    QString version;
    Type type;
    auto operator==(const PluginDependency &other) const -> bool;
    [[nodiscard]] auto toString() const -> QString;
};

struct EXTENSIONSYSTEM_EXPORT PluginArgumentDescription
{
    QString name;
    QString parameter;
    QString description;
};

class EXTENSIONSYSTEM_EXPORT PluginSpec
{
public:
    enum State { Invalid, Read, Resolved, Loaded, Initialized, Running, Stopped, Deleted};

    ~PluginSpec();

    // information from the xml file, valid after 'Read' state is reached
    [[nodiscard]] auto name() const -> QString;
    [[nodiscard]] auto version() const -> QString;
    [[nodiscard]] auto compatVersion() const -> QString;
    [[nodiscard]] auto vendor() const -> QString;
    [[nodiscard]] auto copyright() const -> QString;
    [[nodiscard]] auto license() const -> QString;
    [[nodiscard]] auto description() const -> QString;
    [[nodiscard]] auto longDescription() const -> QString;
    [[nodiscard]] auto url() const -> QString;
    [[nodiscard]] auto category() const -> QString;
    [[nodiscard]] auto revision() const -> QString;
    [[nodiscard]] auto platformSpecification() const -> QRegularExpression;
    [[nodiscard]] auto isAvailableForHostPlatform() const -> bool;
    [[nodiscard]] auto isRequired() const -> bool;
    [[nodiscard]] auto isExperimental() const -> bool;
    [[nodiscard]] auto isEnabledByDefault() const -> bool;
    [[nodiscard]] auto isEnabledBySettings() const -> bool;
    [[nodiscard]] auto isEffectivelyEnabled() const -> bool;
    [[nodiscard]] auto isEnabledIndirectly() const -> bool;
    [[nodiscard]] auto isForceEnabled() const -> bool;
    [[nodiscard]] auto isForceDisabled() const -> bool;
    [[nodiscard]] QVector<PluginDependency> dependencies() const;
    [[nodiscard]] auto metaData() const -> QJsonObject;

    using PluginArgumentDescriptions = QVector<PluginArgumentDescription>;
    [[nodiscard]] PluginArgumentDescriptions argumentDescriptions() const;

    // other information, valid after 'Read' state is reached
    [[nodiscard]] auto location() const -> QString;
    [[nodiscard]] auto filePath() const -> QString;

    [[nodiscard]] auto arguments() const -> QStringList;
    void setArguments(const QStringList &arguments);
    void addArgument(const QString &argument);

    [[nodiscard]] auto provides(const QString &pluginName, const QString &version) const -> bool;

    // dependency specs, valid after 'Resolved' state is reached
    [[nodiscard]] auto dependencySpecs() const -> QHash<PluginDependency, PluginSpec *>;
    [[nodiscard]] auto requiresAny(const QSet<PluginSpec *> &plugins) const -> bool;

    // linked plugin instance, valid after 'Loaded' state is reached
    [[nodiscard]] auto plugin() const -> IPlugin *;

    // state
    [[nodiscard]] auto state() const -> State;
    [[nodiscard]] auto hasError() const -> bool;
    [[nodiscard]] auto errorString() const -> QString;

    void setEnabledBySettings(bool value);

    static auto read(const QString &filePath) -> PluginSpec *;
    static auto read(const QStaticPlugin &plugin) -> PluginSpec *;

private:
    PluginSpec();

    Internal::PluginSpecPrivate *d;
    friend class PluginView;
    friend class Internal::OptionsParser;
    friend class Internal::PluginManagerPrivate;
    friend class Internal::PluginSpecPrivate;
};

} // namespace ExtensionSystem
