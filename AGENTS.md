# AGENTS.md

Qt 6 / C++23 桌面应用模板。`src/extensionsystem/` 是改自 Qt Creator 的插件运行时，
`src/shell/` 是外壳扩展点（`IPage` / `ICommand` / `Host`），`src/plugins/` 下 10 个内置插件
各演示一种写法。面向人的完整说明在 `README.md`。

**这是给人抄的模板，不是产品**：机制与链路讲得清楚优先于把功能做完整；改动要可移植、
两条构建链（CMake / qmake）保持同构，不引入只有本仓库看得懂的抽象。

## 常用命令

Windows 本机（脚本自动进 MSVC 环境，默认 Debug、`C:\Qt\6.12.0\msvc2022_64`）：

```powershell
.\scripts\windows\dev.ps1 -Action configure   # 生成 build\ 与 compile_commands.json
.\scripts\windows\dev.ps1                     # 编译，默认 -Action build
.\scripts\windows\dev.ps1 -Action test        # 构建并跑 ctest
.\scripts\windows\dev.ps1 -Action run -AppArgs '--console','--log-level=debug'
.\scripts\windows\format.ps1                  # clang-format；CI 用 -Check 只检查
```

其它平台：

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DQT_ROOT=/opt/Qt/6.12.0/gcc_64 -DBUILD_TESTS=ON
cmake --build build && ctest --test-dir build --output-on-failure
```

产物统一在 `binaries/cmake/<arch>/` 下：`bin/<config>/`（可运行产物）、`tests/<config>/`。
**交付前**：`dev.ps1` 编译通过 + `format.ps1 -Check` 无差异 + 相关测试通过。

## 动手之前先读

- **改任何 C++/Qt 源文件前，读 `docs/coding-standards.md`**——PIMPL 规则、命名、信号槽、
  `Utils::Result` 这些"看代码猜不到"的约定全在那里，本文不复述。
- **改主题 / QSS 前，读 `docs/theming.md`**。
- 插件机制与生命周期、新增插件的步骤：`README.md` 的「插件系统」「新增一个插件」两节。

## 陷阱

1. **版本号唯一来源是 `packaging/version.txt`**。改版本必须同步 `src/utils/appinfo.hpp` 的
   `version = "x.y.z"`（写法不能变），否则 configure 阶段直接 FATAL；`vcpkg.json` 的
   `version` 不一致只会 WARNING，但也要一起改。
2. **源文件与插件的增删要双链登记**：同时改 `CMakeLists.txt` 与同名 `.pro`。目标名 = 目录名，
   禁止 `file(GLOB)`。
3. **插件元数据三处必须一致**：`<目录名>.json` 的 `Id` = 目录名 = 目标名，`Version` = version.txt，
   头文件里 `Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "<目录名>.json")`。新插件从
   `src/plugins/helloplugin/` 复制，并在 `src/plugins/CMakeLists.txt` 与 `plugins.pro` 各加一行。
4. **测试**：默认不构建（`BUILD_TESTS=OFF`）；目标用 `tst_` 前缀（唯一的命名例外）；产物落在
   `binaries/cmake/<arch>/tests/`，**不能落 `bin/`、不写 `install()`**——debian 链会把
   `binaries/cmake/*/bin/*/` 整个打进 `.deb`。新增测试目录一行接入：
   `add_app_test(tst_xxx SOURCES xxx.cpp LIBRARIES <模块>)`。
5. **两条构建链同构**：改 `cmake/*.cmake` 时对照 `qmake/*.pri` 的同名文件。qmake 只做编译验证，
   不参与打包。
6. **`.ps1` 一律纯 ASCII（UTF-8 无 BOM）**：Windows PowerShell 5.1 对无 BOM 脚本按 ANSI 码页
   解码，脚本里写 UTF-8 中文会直接解析失败。注释与提示用英文，中文说明放 Markdown；要写中文就得
   同时加 UTF-8 BOM。
