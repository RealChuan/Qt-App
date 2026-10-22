// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "appinfo.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QString>

namespace Utils {

namespace {

// 目录可能被用户或清理工具删掉，所以每次取路径都要重新确保它存在。
auto ensureDirectory(const QString &path) -> QString
{
    QDir().mkpath(path);
    return path;
}

auto buildAppInfo() -> AppInfo
{
    const FilePath appDirPath = FilePath::fromUserInput(QCoreApplication::applicationDirPath());
    const QString pluginDirName = QString::fromLatin1(appName).toLower();

    AppInfo info;
#ifdef Q_OS_MACOS
    info.plugins = (appDirPath / "../PlugIns" / pluginDirName).cleanPath();
    info.resources = (appDirPath / "../Resources").cleanPath();
#else
    info.plugins = (appDirPath / "plugins" / pluginDirName).cleanPath();
    info.resources = (appDirPath / "resources").cleanPath();
#endif
    return info;
}

} // namespace

auto appInfo() -> const AppInfo &
{
    static const AppInfo info = buildAppInfo();
    return info;
}

auto configPath() -> QString
{
    return ensureDirectory(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
}

auto configFilePath() -> QString
{
    return QDir(configPath()).filePath(QStringLiteral("config.ini"));
}

auto logPath() -> QString
{
    // 日志属于应用状态，不是配置
    return ensureDirectory(QDir(QStandardPaths::writableLocation(QStandardPaths::StateLocation))
                               .filePath(QStringLiteral("log")));
}

auto crashPath() -> QString
{
    return ensureDirectory(QDir(QStandardPaths::writableLocation(QStandardPaths::StateLocation))
                               .filePath(QStringLiteral("crash")));
}

auto cachePath() -> QString
{
    return ensureDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
}

} // namespace Utils
