# Qt-App

[English](README.en.md)English version by[readme.yml](.github/workflows/readme.yml)Automatic translation.

> The picture material comes from the Internet and is only used for learning and demonstration; for commercial use, please confirm the material authorization yourself. code to follow[GPL-3.0](LICENSE)。

one**Qt desktop application templates you can copy**: The function is just a demonstration. What I really want to explain are three things - how to load the plug-in, how to maintain the isomorphism of the two build chains, and how to complete the process from compilation to installation package.

| what do you want to see  | Where to enter                                                                                 |
| ------------------------ | ---------------------------------------------------------------------------------------------- |
| Plug-in mechanism        | [src/extensionsystem/](src/extensionsystem)(runtime) +[src/shell/](src/shell)(Extension point) |
| Two construction chains  | [CMakeLists.txt](CMakeLists.txt)/[cmake/](cmake)and[Qt-App.pro](Qt-App.pro)/[qmake/](qmake)    |
| Packaging and publishing | [packaging/](packaging)+[.github/workflows/](.github/workflows)                                |

## Preview

<img src="docs/images/app.png" width="100%">

## Directory structure

| path                                        | content                                                                                                                                                                                                                                              |
| ------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [src/extensionsystem/](src/extensionsystem) | When the plugin is running:[iplugin.hpp](src/extensionsystem/iplugin.hpp)、[pluginmanager.hpp](src/extensionsystem/pluginmanager.hpp)、[pluginspec.hpp](src/extensionsystem/pluginspec.hpp)、[plugincontext.hpp](src/extensionsystem/plugincontext.hpp) |
| [src/shell/](src/shell)                     | Shell extension points:[page.hpp](src/shell/page.hpp)、[command.hpp](src/shell/command.hpp)、[host.hpp](src/shell/host.hpp)、[registry.hpp](src/shell/registry.hpp)                                                                                     |
| [src/plugins/](src/plugins)                 | Built-in plug-ins,[helloplugin](src/plugins/helloplugin)is the minimal template                                                                                                                                                                      |
| [src/apps/app/](src/apps/app/main.cc)       | `main()`: Single instance, log, theme, Crashpad initialization, crash prompt, plug-in loading                                                                                                                                                        |
| [src/utils/](src/utils)                     | Themes, settings, asynchronous logs,`FilePath`、`Result`and other infrastructure; the auxiliary functions to touch the control are in[uihelpers.hpp](src/utils/uihelpers.hpp)                                                                         |
| [src/widgets/](src/widgets)                 | Common controls and dialog boxes                                                                                                                                                                                                                     |
| [src/resource/](src/resource)               | qrc[QSS and topic tokens](src/resource/qss),icon                                                                                                                                                                                                     |
| [src/crashpadclient/](src/crashpadclient)   | Crashpad client package                                                                                                                                                                                                                              |
| [src/3rdparty/](src/3rdparty)               | QtSingleApplication                                                                                                                                                                                                                                  |
| [cmake/](cmake)·[qmake/](qmake)             | Two modules for building the chain, one-to-one correspondence                                                                                                                                                                                        |
| [packaging/](packaging)                     | Three-platform packaging script, version number, release and upload                                                                                                                                                                                  |
| [tests/](tests)                             | QTest unit test (only use CMake, not built by default)                                                                                                                                                                                               |
| [translations/](translations)               | `.ts`Translate source files                                                                                                                                                                                                                          |
| [docs/](docs)                               | [theme](docs/theming.md)、[Coding standards](docs/coding-standards.md)                                                                                                                                                                                |

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
| `PluginContext`     | [plugincontext.hpp](src/extensionsystem/plugincontext.hpp) | `initialize()` 里拿 `settings()`、`resourcePath()`                         |
| `IPage`             | [page.hpp](src/shell/page.hpp)                             | Sidebar page (Pages/Games/Settings/Help)                                |
| `ICommand`          | [command.hpp](src/shell/command.hpp)                       | command panel (`Ctrl+K`) actions in                                     |
| `Host`              | [host.hpp](src/shell/host.hpp)                             | Main window, page switching, status bar prompts, restart                |
| Service object pool | [pluginmanager.hpp](src/extensionsystem/pluginmanager.hpp) | `addObject()`/`object<T>()`, get services by interface between plug-ins |

The built-in plug-ins each demonstrate one way of writing:

| plug-in                                            | Demo content                                                                         |
| -------------------------------------------------- | ------------------------------------------------------------------------------------ |
| [coreplugin](src/plugins/coreplugin)               | Shell implementation: main window, command panel, plug-in list (`Required: true`）    |
| [helloplugin](src/plugins/helloplugin)             | Minimal template: metadata + one page + one command + persistence settings           |
| [particlesplugin](src/plugins/particlesplugin)     | Self-drawn control + timer-driven frame animation                                    |
| [danmakuplugin](src/plugins/danmakuplugin)         | Service Provider: Register[`IDanmaku`](src/api/danmaku/idanmaku.hpp)                 |
| [pomodoroplugin](src/plugins/pomodoroplugin)       | Service consumer: downgrade to status bar prompt when optional dependency is missing |
| [fractalplugin](src/plugins/fractalplugin)         | Background calculation + cancel                                                      |
| [colorpickerplugin](src/plugins/colorpickerplugin) | Independent tool window + screen color selection                                     |
| [snakeplugin](src/plugins/snakeplugin)             | Mini-games: Fixed step loop, keyboard focus, highest score archive                   |
| [tetrisplugin](src/plugins/tetrisplugin)           | Mini games: 60 Hz frame loop, SRS wall kick, DAS/ARR burst                           |

### Add a new plugin

1.  copy[src/plugins/helloplugin/](src/plugins/helloplugin)And renamed it to the new plug-in directory;
2.  Change metadata`<id>.json`：`Id`Must be equal to directory name and target name,`Version`and[packaging/version.txt](packaging/version.txt)consistent; consistent;
3.  Write in header file`Q_PLUGIN_METADATA(IID "Youth.Qt.plugin" FILE "<id>.json")`,accomplish`IPlugin`；
4.  Register at the same time[src/plugins/CMakeLists.txt](src/plugins/CMakeLists.txt)and[src/plugins/plugins.pro](src/plugins/plugins.pro)；
5.  `initialize()`Only create objects,`extensionsInitialized()`Used inside`Shell::Registry::instance()`Registration page and commands.

CMake side by side[add_app_plugin()](cmake/PlatformLibraries.cmake)Unified processing of plug-in suffix, output directory and RPATH, and verification of json`Id`/`Version`。

## build

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

Windows native machine can be used directly[scripts/windows/dev.ps1](scripts/windows/dev.ps1)（[illustrate](scripts/windows/README.md)), it will advance the MSVC development environment:

```powershell
.\scripts\windows\dev.ps1 -Action configure
.\scripts\windows\dev.ps1                                   # 编译
.\scripts\windows\dev.ps1 -Action run -AppArgs '--console','--log-level=debug'
```

The product is in`binaries/cmake/<arch>/bin/<config>/`(Main program +`plugins/qt-app/`+`resources/`+`translations/`), see layout[OutputDirectories.cmake](cmake/OutputDirectories.cmake)。

### Unit testing

QTest + CTest, not built by default; the product is`binaries/cmake/<arch>/tests/<config>/`, does not participate in installation and packaging.

```powershell
.\scripts\windows\dev.ps1 -Action test
```

Other platforms: add when configure`-DBUILD_TESTS=ON`,Again`ctest --test-dir build --output-on-failure`. See the test directory structure[tests/](tests)。

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

## Packaging and publishing

There is only one source for version numbers:[packaging/version.txt](packaging/version.txt). CMake, qmake and all packaging scripts read it,`vcpkg.json`in`version`Inconsistencies will alert you during configuration.

| platform                       | product                                    | Entrance                                                                                                                                                                       |
| ------------------------------ | ------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Windows                        | `*.7z`Carrying case + Inno Setup installer | [packaging/windows/package.ps1](packaging/windows/package.ps1)、[app.iss](packaging/windows/app.iss)                                                                            |
| macOS                          | x86_64/arm64/universal`.dmg`、`.pkg`、`.7z`  | [packaging/macos/package.sh](packaging/macos/package.sh), the universal binary consists of[merge_universal_app.sh](packaging/macos/app_arch_tools/merge_universal_app.sh)merge |
| Ubuntu                         | `.AppImage`、`.deb`、`.7z`                   | [packaging/ubuntu/package.sh](packaging/ubuntu/package.sh)                                                                                                                     |
| Ubuntu/Debian official package | debhelper process output`.deb`             | [packaging/debian/rules](packaging/debian/rules)+[debian.yml](.github/workflows/debian.yml)                                                                                    |
| Self-built update source       | Products for each platform +`update.json`  | [packaging/upload.py](packaging/upload.py)                                                                                                                                     |

The scripts for the three platforms all write the product into`packaging/releases/`,Depend on[flatten_releases.sh](packaging/flatten_releases.sh)Publish after tiling. CI division of labor:

| Workflow                                         | effect                                                                                     |
| ------------------------------------------------ | ------------------------------------------------------------------------------------------ |
| [cmake.yml](.github/workflows/cmake.yml)         | Three platforms are built and packaged, and Release is automatically released when tagging |
| [test.yml](.github/workflows/test.yml)           | Run QTest unit tests on three platforms (not packaged)                                     |
| [qmake.yml](.github/workflows/qmake.yml)         | Compilation verification of qmake chain                                                    |
| [debian.yml](.github/workflows/debian.yml)       | Debian official package                                                                    |
| [toolchain.yml](.github/workflows/toolchain.yml) | Pre-built dependency cache                                                                 |
| [readme.yml](.github/workflows/readme.yml)       | Translate this document into[README.en.md](README.en.md)                                   |

## Themes and Internationalization

-   **theme**: One set of QSS + two`.theme`Token file, regenerate the style sheet and synchronize the color palette when switching between dark and light colors, see[docs/theming.md](docs/theming.md)。
-   **internationalization**: The translation file is in[translations/](translations)，`cmake --build build --target Qt-App_lupdate`renew,`Qt-App_lrelease`Publish; the current language needs to be restarted after switching to take effect. See the real-time refresh writing method.`changeEvent(QEvent::LanguageChange)`+`retranslateUi()`。

## crash report

Crashpad is accessed through "in-process client + independent handler", see[src/crashpadclient/](src/crashpadclient): When the handler crashes, write the dmp into`<AppConfigLocation>/crash`，`reportUrl`If you pass it empty, it will only be placed on the disk and not uploaded.

The process has already exited when it crashes, so the prompt can only be caused by**next boot**issue:`main()`Check the pending reports in the crashpad database before loading the plug-in.
If there is one, it will pop up[crashdialog](src/apps/app/crashdialog.cc). After the user closes the report, it will be marked as completed and will not be prompted repeatedly; selecting "Restart" will reuse the main program.
The existing restart mechanism is used instead of starting a new process.

Known boundaries:`StartHandler`Previous (very early) crashes will not produce a dump and therefore will not prompt.

## FAQ

-   **Plugin not loaded**:Watch first`*.json`of`Id`Is it equal to the library file name?`Version`whether with[version.txt](packaging/version.txt)consistent; the plugin directory should be located next to the executable`plugins/qt-app/`。
-   **Windows packaging**:need`7z`exist`PATH`, Inno Setup is installed, and passed[Enter-VsDevShell.ps1](packaging/windows/Enter-VsDevShell.ps1)Enter the MSVC environment.
-   **macOS unsigned**: The installation package is not signed/notarized by default, and ad-hoc signature is required for local running:`codesign --force --deep --sign - Qt-App.app`。
-   **Ubuntu running**:need`libfuse2`、`libxcb-cursor0`;Can be set when the window behaves abnormally under Wayland`QT_QPA_PLATFORM=xcb`。
-   **No prompts were seen after the crash**：`crashpad_handler`Must have execute permission (`chmod +x`); Prompt after the crash**next boot**Just appeared,
    There will be no pop-up window when there is no new dmp.
-   **User data remaining after uninstallation**: Windows installation package and Linux`.deb`Configuration/log/crash/cache will be deleted during uninstallation;
    macOS`.pkg`No uninstall hook (drag away`.app`(that’s it), you need to clean it manually`~/Library/Preferences/Youth/Qt-App`、`~/Library/Caches/Youth/Qt-App`.
    For the specific placement of user data, see[src/utils/appinfo.hpp](src/utils/appinfo.hpp)annotation.

## document

-   [docs/coding-standards.md](docs/coding-standards.md): Naming, PIMPL, signals, slots and pre-submit checks
-   [docs/theming.md](docs/theming.md): Token replacement and porting to other Qt programs
-   [scripts/windows/README.md](scripts/windows/README.md): Native development scripts with`.ps1`Coding conventions
