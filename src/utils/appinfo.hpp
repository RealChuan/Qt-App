// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.hpp"

#include "filepath.hpp"

namespace Utils {

// 应用级身份与路径。全部是静态成员，调用方不需要持有 AppInfo 实例。
//
// version 必须与 packaging/version.txt 一致：打包脚本、CI、vcpkg 清单都读那个文件，
// 而 C++ 侧读不了它。根 CMakeLists.txt 在配置期校验两者是否相同，不一致直接报错，
// 因此变量名和 `version = "x.y.z"` 的写法不能改。
struct UTILS_EXPORT AppInfo
{
    // -------------------------------------------------------------------------
    // 编译期身份
    // -------------------------------------------------------------------------
    static constexpr auto version = "0.2.1";
    static constexpr auto appName = "Qt-App";
    static constexpr auto organizationName = "Youth";
    static constexpr auto organizationDomain = "Youth";
    static constexpr auto copyright = "Copyright 2017-2026 Youth. All rights reserved.";

    // -------------------------------------------------------------------------
    // 运行期安装布局：只有可执行文件位置才能算出来的路径
    //
    // 首次调用时计算并缓存；依赖 QCoreApplication::applicationDirPath()，
    // 因此必须在 QCoreApplication 构造之后调用（插件加载时已满足）。
    // -------------------------------------------------------------------------

    // <exe>/plugins/<appName>（macOS: <app>.app/Contents/PlugIns/<appName>）
    [[nodiscard]] static auto plugins() -> const FilePath &;
    // <exe>/resources（macOS: <app>.app/Contents/Resources）
    [[nodiscard]] static auto resources() -> const FilePath &;

    // -------------------------------------------------------------------------
    // 用户数据目录（可写，与安装目录分开）
    //
    // 直接对应 QStandardPaths 的 app 专属位置（StateLocation 需要 Qt 6.7+）：
    //   config  AppConfigLocation    macOS  ~/Library/Preferences/<app>
    //                                Linux  ~/.config/<app>
    //                                Windows %LOCALAPPDATA%\<app>
    //   log     StateLocation/log    macOS  ~/Library/Preferences/<app>/State/log
    //                                Linux  ~/.local/state/<app>/log
    //                                Windows %LOCALAPPDATA%\<app>\State\log
    //   crash   StateLocation/crash  同上，末级换成 crash
    //   cache   CacheLocation        macOS  ~/Library/Caches/<app>
    //                                Linux  ~/.cache/<app>
    //                                Windows %LOCALAPPDATA%\<app>\cache
    //
    // Qt 会在 <app> 前补上 organizationName（Windows 实测为 Youth\Qt-App），
    // 所以卸载脚本按 <org>/<app> 清理即可，不需要猜别的形态。
    //
    // 这些目录可能被用户或清理工具删掉，所以**每次调用都会重新确保目录存在**；
    // mkpath 对已存在的目录只是一次 stat，代价可以忽略。调用方不必自己再建目录。
    // -------------------------------------------------------------------------
    [[nodiscard]] static auto configPath() -> FilePath;
    [[nodiscard]] static auto configFilePath() -> FilePath;
    [[nodiscard]] static auto crashPath() -> FilePath;
    [[nodiscard]] static auto logPath() -> FilePath;
    [[nodiscard]] static auto cachePath() -> FilePath;
};

} // namespace Utils
