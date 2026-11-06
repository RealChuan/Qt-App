// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "appinfo.hpp"

#include <QCoreApplication>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QString>

using namespace Qt::StringLiterals;

namespace Utils {

namespace {

auto appDirPath() -> FilePath
{
    return FilePath::fromUserInput(QCoreApplication::applicationDirPath());
}

auto appNameLower() -> QString
{
    return QString::fromLatin1(AppInfo::appName).toLower();
}

// 安装前缀。规则与 Qt 自己找插件一致：可执行文件旁的 qt.conf 优先，
// 没有就退回可执行文件目录（构建树）。Windows 的 qt.conf 写 Prefix=.，
// 解出来同样是可执行文件目录。
auto buildPrefixPath() -> FilePath
{
#ifdef Q_OS_MACOS
    // .app/Contents（bundle 里的 qt.conf 不写 Prefix，不能拿来当依据）
    return (appDirPath() / "..").cleanPath();
#else
    if (QFileInfo::exists(QCoreApplication::applicationDirPath() + u"/qt.conf"_s)) {
        // Prefix 是相对 qt.conf 所在目录解析的，QLibraryInfo 已经算好了
        return FilePath::fromUserInput(QLibraryInfo::path(QLibraryInfo::PrefixPath));
    }
    return appDirPath();
#endif
}

auto buildPluginsPath() -> FilePath
{
#ifdef Q_OS_MACOS
    return (AppInfo::prefix() / "PlugIns" / appNameLower()).cleanPath();
#else
    return (AppInfo::prefix() / "plugins" / appNameLower()).cleanPath();
#endif
}

auto buildResourcesPath() -> FilePath
{
#ifdef Q_OS_MACOS
    return (AppInfo::prefix() / "Resources").cleanPath();
#elif defined(Q_OS_LINUX)
    return (AppInfo::prefix() / "share" / appNameLower() / "resources").cleanPath();
#else
    return (AppInfo::prefix() / "resources").cleanPath();
#endif
}

// 目录可能被用户或清理工具删掉，所以每次取路径都要重新确保它存在。
auto ensureDirectory(const FilePath &path) -> FilePath
{
    path.createDir();
    return path;
}

} // namespace

auto AppInfo::prefix() -> const FilePath &
{
    static const FilePath path = buildPrefixPath();
    return path;
}

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
    return configPath() / u"config.ini"_s;
}

auto AppInfo::logPath() -> FilePath
{
    // 日志属于应用状态，不是配置
    return ensureDirectory(
        FilePath(QStandardPaths::writableLocation(QStandardPaths::StateLocation)) / u"log"_s);
}

auto AppInfo::crashPath() -> FilePath
{
    return ensureDirectory(
        FilePath(QStandardPaths::writableLocation(QStandardPaths::StateLocation)) / u"crash"_s);
}

auto AppInfo::cachePath() -> FilePath
{
    return ensureDirectory(
        FilePath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));
}

} // namespace Utils
