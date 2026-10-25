# Qt-App

[English](README.en.md)English version by[readme.yml](.github/workflows/readme.yml)Automatic translation.

one**Qt desktop application templates you can copy**: The function is just a demonstration. What I really want to explain are three things - how to load the plug-in, how to maintain the isomorphism of the two build chains, and how to complete the process from compilation to installation package.

> The picture materials are from the Internet and are only used for learning and demonstration; for commercial use, please confirm the material authorization yourself. code to follow[GPL-3.0](LICENSE)。

<img src="docs/images/app.png" width="100%">

## Where to enter

| what do you want to see  | Where to enter                                                                                 |
| ------------------------ | ---------------------------------------------------------------------------------------------- |
| Plug-in mechanism        | [src/extensionsystem/](src/extensionsystem)(runtime) +[src/shell/](src/shell)(Extension point) |
| Two construction chains  | [CMakeLists.txt](CMakeLists.txt)/[cmake/](cmake)and[Qt-App.pro](Qt-App.pro)/[qmake/](qmake)    |
| Packaging and publishing | [packaging/](packaging)+[.github/workflows/](.github/workflows)                                |
| Topics and Norms         | [docs/theming.md](docs/theming.md)、[docs/coding-standards.md](docs/coding-standards.md)        |

## Directory structure

`src/plugins/`Each subdirectory under it is a plugin,[helloplugin](src/plugins/helloplugin)is the minimal template; the rest of the directories need explanation:

| path                                        | content                                                                                                                                           |
| ------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| [src/extensionsystem/](src/extensionsystem) | When the plugin is running:[iplugin.hpp](src/extensionsystem/iplugin.hpp)、[pluginmanager.hpp](src/extensionsystem/pluginmanager.hpp)              |
| [src/shell/](src/shell)                     | Shell extension points:[page.hpp](src/shell/page.hpp)、[command.hpp](src/shell/command.hpp)、[host.hpp](src/shell/host.hpp)                         |
| [src/apps/app/](src/apps/app/main.cc)       | `main()`: Single instance, log, theme, Crashpad, crash prompt, plug-in loading                                                                    |
| [src/utils/](src/utils)                     | Themes, settings, asynchronous logs,`FilePath`、`Result`;The auxiliary function to touch the control is in[uihelpers.hpp](src/utils/uihelpers.hpp) |
| [src/widgets/](src/widgets)                 | Common controls and dialog boxes                                                                                                                  |
| [src/resource/](src/resource)               | qrc[QSS and topic tokens](src/resource/qss),icon                                                                                                  |
| [src/crashpadclient/](src/crashpadclient)   | Crashpad client package                                                                                                                           |
| [cmake/](cmake)·[qmake/](qmake)             | Two modules for building the chain, one-to-one correspondence                                                                                     |
| [packaging/](packaging)                     | Three-platform packaging script, version number, release and upload                                                                               |
| [tests/](tests)                             | QTest unit test (only use CMake, not built by default)                                                                                            |
| [translations/](translations)               | `.ts`Translate source files                                                                                                                       |

## Plug-in mechanism

Plug-in system modified from[Qt Creator](https://github.com/qt-creator/qt-creator/tree/master/src/libs/extensionsystem), retaining the three-piece set of "metadata + dependency + life cycle", and removing the part that is strongly bound to the IDE.

```text
plugins/<模块名>.dll|so|dylib
  └─ 读 Q_PLUGIN_METADATA 里内嵌的 <id>.json
       └─ 解析 Dependencies（Required / Optional）并按依赖排序
            └─ load → initialize(context)      拿本插件专属 QSettings 与资源根
                 └─ extensionsInitialized()    注册页面/命令；此时其它插件的服务已就绪
                      └─ aboutToShutdown()     与初始化相反的顺序
```

| extension point     | definition                                                 | use                                                                     |
| ------------------- | ---------------------------------------------------------- | ----------------------------------------------------------------------- |
| `IPlugin`           | [iplugin.hpp](src/extensionsystem/iplugin.hpp)             | Plug-in entrance,`IID`fixed to`Youth.Qt.plugin`                         |
| `PluginContext`     | [plugincontext.hpp](src/extensionsystem/plugincontext.hpp) | `initialize()`Lina`settings()`、`resourcePath()`                         |
| `IPage`             | [page.hpp](src/shell/page.hpp)                             | Sidebar page (Pages/Games/Settings/Help)                                |
| `ICommand`          | [command.hpp](src/shell/command.hpp)                       | command panel (`Ctrl+K`) actions in                                     |
| `Host`              | [host.hpp](src/shell/host.hpp)                             | Main window, page switching, status bar prompts, restart                |
| Service object pool | [pluginmanager.hpp](src/extensionsystem/pluginmanager.hpp) | `addObject()`/`object<T>()`, get services by interface between plug-ins |

The built-in plug-ins each demonstrate one way of writing:

| plug-in                                            | Demo content                                                                      |
| -------------------------------------------------- | --------------------------------------------------------------------------------- |
| [coreplugin](src/plugins/coreplugin)               | Shell implementation: main window, command panel, plug-in list (`Required: true`） |
| [helloplugin](src/plugins/helloplugin)             | Minimal template: page + command + persistence settings                           |
| [particlesplugin](src/plugins/particlesplugin)     | Self-drawn control + timer frame animation                                        |
| [danmakuplugin](src/plugins/danmakuplugin)         | Service provider:[`IDanmaku`](src/api/danmaku/idanmaku.hpp)                       |
| [pomodoroplugin](src/plugins/pomodoroplugin)       | Service consumer: downgrade when optional dependencies are missing                |
| [fractalplugin](src/plugins/fractalplugin)         | Background calculation + cancel                                                   |
| [colorpickerplugin](src/plugins/colorpickerplugin) | Independent tool window + screen color selection                                  |
| [snakeplugin](src/plugins/snakeplugin)             | Fixed step loop, keyboard focus, high score archiving                             |
| [tetrisplugin](src/plugins/tetrisplugin)           | 60 Hz frame loop, SRS wall kick, DAS/ARR burst                                    |
| [fortuneplugin](src/plugins/fortuneplugin)         | Pure logic hexagram engine, 1901–2100 lunar calendar, Qt animation reveal         |

### Add a new plugin

1.  copy[src/plugins/helloplugin/](src/plugins/helloplugin)And renamed it to the new plug-in directory;
2.  Change metadata`<id>.json`：`Id`Must be equal to directory name and target name,`Version`and[packaging/version.txt](packaging/version.txt)consistent; consistent;
3.  Write in header file`Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "<id>.json")`,accomplish`IPlugin`；
4.  Register at the same time[src/plugins/CMakeLists.txt](src/plugins/CMakeLists.txt)and[src/plugins/plugins.pro](src/plugins/plugins.pro)；
5.  `initialize()`Only create objects,`extensionsInitialized()`Used inside`Shell::Registry::instance()`Registration page and commands.

> CMake side by side[add_app_plugin()](cmake/PlatformLibraries.cmake)Unified processing of plug-in suffix, output directory and RPATH, and verification of json`Id`/`Version`。

## Build

| rely     | Require                                                                                                                             |
| -------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| Qt       | 6.12 (default searches for 6.12.0 paths on each platform), available`-DQT_ROOT=<dir>`or environment variables`QT_ROOT`cover         |
| compiler | C++23：MSVC 19.34+ / GCC 12+ / Clang 15+                                                                                             |
| CMake    | ≥ 3.25.1 with Ninja                                                                                                                 |
| vcpkg    | default`C:/vcpkg`or`/usr/local/share/vcpkg`, available`-DVCPKG_DEFAULT_ROOT`Change; depend on see[vcpkg.json](vcpkg.json)（crashpad） |

### CMake (used for main chain, packaging and CI)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQT_ROOT=/opt/Qt/6.12.0/gcc_64
cmake --build build
cmake --install build --prefix dist   # 可选：安装到前缀目录（Windows/macOS 顺带部署 Qt 依赖）
```

The product is in`binaries/cmake/<arch>/bin/<config>/`(Main program +`plugins/qt-app/`+`resources/`+`translations/`), see layout[OutputDirectories.cmake](cmake/OutputDirectories.cmake)。

> For Windows local machine[scripts/windows/dev.ps1](scripts/windows/dev.ps1)That’s it, parameters, product paths and`.ps1`For coding conventions see[scripts/windows/README.md](scripts/windows/README.md)。

### Unit testing

QTest + CTest, not built by default; the product is`binaries/cmake/<arch>/tests/<config>/`, does not participate in installation and packaging. Directory structure see[tests/](tests)。

```powershell
.\scripts\windows\dev.ps1 -Action test
```

Other platforms: add when configure`-DBUILD_TESTS=ON`,Again`ctest --test-dir build --output-on-failure`。

### qmake (standby chain, only compilation verification)

```bash
mkdir build && cd build
qmake ../. && make -j
```

The product is in`binaries/qmake/<arch>/bin/{debug,release}/`; Dependencies are installed in the vcpkg root directory in classic mode, see[qmake/VcpkgToolchain.pri](qmake/VcpkgToolchain.pri). Packaging does not use this chain.

### Common CMake options

| Options                        | default               | illustrate                                                               |
| ------------------------------ | --------------------- | ------------------------------------------------------------------------ |
| `-DQT_ROOT=<dir>`              | Platform default path | Qt prefix                                                                |
| `-DVCPKG_TARGET_TRIPLET=<t>`   | by platform           | Consistent with qmake side                                               |
| `-DUSE_VCPKG=OFF`              | `ON`                  | You need to provide it yourself after closing`crashpad`The CMake package |
| `-DUSE_UNITY_BUILD=ON`         | `OFF`                 | unity/jumbo build                                                        |
| `-DRELEASE_WITH_DEBUGINFO=OFF` | `ON`                  | Does Release contain debugging information?                              |
| `-DBUILD_TESTS=ON`             | `OFF`                 | Build QTest unit tests (no installation, no packaging)                   |

## 打包与发布

There is only one source for version numbers:[packaging/version.txt](packaging/version.txt). CMake, qmake, and all packaging scripts read it.

> [vcpkg.json](vcpkg.json)inside`version`If it is inconsistent with it, a warning will be issued during the configuration phase.

| platform                       | product                                    | Entrance                                                                                                                                                                       |
| ------------------------------ | ------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Windows                        | `*.7z`Carrying case + Inno Setup installer | [packaging/windows/package.ps1](packaging/windows/package.ps1)、[app.iss](packaging/windows/app.iss)                                                                            |
| macOS                          | x86_64/arm64/universal`.dmg`、`.pkg`、`.7z`  | [packaging/macos/package.sh](packaging/macos/package.sh), the universal binary consists of[merge_universal_app.sh](packaging/macos/app_arch_tools/merge_universal_app.sh)merge |
| Ubuntu                         | `.AppImage`、`.deb`、`.7z`                   | [packaging/ubuntu/package.sh](packaging/ubuntu/package.sh)                                                                                                                     |
| Ubuntu/Debian official package | debhelper process output`.deb`             | [packaging/debian/rules](packaging/debian/rules)+[debian.yml](.github/workflows/debian.yml)                                                                                    |
| Self-built update source       | Products for each platform +`update.json`  | [packaging/upload.py](packaging/upload.py)                                                                                                                                     |

The scripts for the three platforms all write the product into`packaging/releases/`,Depend on[flatten_releases.sh](packaging/flatten_releases.sh)Publish after tiling. CI divides work according to responsibilities:[cmake.yml](.github/workflows/cmake.yml)Run unit testing and build packaging in parallel, and release only when both are green when tagging.[qmake.yml](.github/workflows/qmake.yml)Verify backup chain,[debian.yml](.github/workflows/debian.yml)Release official package,[toolchain.yml](.github/workflows/toolchain.yml)Cache dependencies.

## Themes and Internationalization

-   **theme**: One set of QSS + two`.theme`Token file, regenerate the style sheet and synchronize the palette when switching between dark and light colors, see[docs/theming.md](docs/theming.md)。
-   **internationalization**: The translation file is in[translations/](translations)，`cmake --build build --target Qt-App_lupdate`renew,`Qt-App_lrelease`release.

> After switching the language, you need to restart to take effect; the real-time refresh method is`changeEvent(QEvent::LanguageChange)`+`retranslateUi()`。

## crash report

Crashpad is accessed through "in-process client + independent handler", see[src/crashpadclient/](src/crashpadclient): When the handler crashes, write the dmp into`<AppConfigLocation>/crash`，`reportUrl`If you pass it empty, it will only be placed on the disk and not uploaded.

The process has already exited when it crashes, so the prompt can only be caused by**next boot**issue:`main()`Before loading the plug-in, check the pending reports in the database and pop up if any.[crashdialog](src/apps/app/crashdialog.cc). After the user closes the report, the report will be marked as completed and will not be prompted repeatedly; selecting "Restart" will reuse the restart mechanism of main instead of starting a new process.

> Known boundaries:`StartHandler`Previous (very early) crashes will not produce a dump and therefore will not prompt.

## FAQ

-   **Plugin not loaded**:look`*.json`of`Id`Is it equal to the library file name?`Version`whether with[version.txt](packaging/version.txt)consistent; the plugin directory should be located next to the executable`plugins/qt-app/`。
-   **Windows packaging**:need`7z`exist`PATH`, Inno Setup is installed, and passed[Enter-VsDevShell.ps1](packaging/windows/Enter-VsDevShell.ps1)Enter the MSVC environment.
-   **Ubuntu running**:need`libfuse2`、`libxcb-cursor0`; Set when the window behaves abnormally under Wayland`QT_QPA_PLATFORM=xcb`。
-   **macOS unsigned**: The installation package is not signed/notarized by default, and ad-hoc signature is required for local running:`codesign --force --deep --sign - Qt-App.app`。
-   **No prompts were seen after the crash**：`crashpad_handler`Must have execute permission (`chmod +x`), and the prompt will only appear on the next startup.
-   **User data remaining after uninstallation**: Windows installation package and Linux`.deb`Will delete configuration/log/crash/cache during uninstall; macOS`.pkg`No uninstall hook (drag away`.app`(that’s it), you need to clean it manually`~/Library/Preferences/Youth/Qt-App`、`~/Library/Caches/Youth/Qt-App`. See the location of user data[src/utils/appinfo.hpp](src/utils/appinfo.hpp)annotation.
