// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "appinfo.hpp"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QString>

namespace Utils {

namespace {

auto appDirPath() -> FilePath
{
    return FilePath::fromUserInput(QCoreApplication::applicationDirPath());
}

auto buildPluginsPath() -> FilePath
{
    const QString pluginDirName = QString::fromLatin1(AppInfo::appName).toLower();
#ifdef Q_OS_MACOS
    return (appDirPath() / "../PlugIns" / pluginDirName).cleanPath();
#else
    return (appDirPath() / "plugins" / pluginDirName).cleanPath();
#endif
}

auto buildResourcesPath() -> FilePath
{
#ifdef Q_OS_MACOS
    return (appDirPath() / "../Resources").cleanPath();
#else
    return (appDirPath() / "resources").cleanPath();
#endif
}

// 目录可能被用户或清理工具删掉，所以每次取路径都要重新确保它存在。
auto ensureDirectory(const FilePath &path) -> FilePath
{
    path.createDir();
    return path;
}

} // namespace

auto AppInfo::plugins() -> const FilePath &
{
    static const FilePath path = buildPluginsPath();
    return path;
}

auto AppInfo::resources() -> const FilePath &
{
    static const FilePath path = buildResourcesPath();
    return path;
}

auto AppInfo::configPath() -> FilePath
{
    return ensureDirectory(
        FilePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)));
}

auto AppInfo::configFilePath() -> FilePath
{
    return configPath() / QStringLiteral("config.ini");
}

auto AppInfo::logPath() -> FilePath
{
    // 日志属于应用状态，不是配置
    return ensureDirectory(
        FilePath(QStandardPaths::writableLocation(QStandardPaths::StateLocation))
        / QStringLiteral("log"));
}

auto AppInfo::crashPath() -> FilePath
{
    return ensureDirectory(
        FilePath(QStandardPaths::writableLocation(QStandardPaths::StateLocation))
        / QStringLiteral("crash"));
}

auto AppInfo::cachePath() -> FilePath
{
    return ensureDirectory(
        FilePath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));
}

} // namespace Utils
