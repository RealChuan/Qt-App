# Qt-App

- [简体中文](README.md)
- [English](README.en.md)

> 图片资源等来自于互联网。  
> 本代码仓库仅供学习使用，若被用于商业用途与本人无关！请遵守相关许可证。

## 项目简介

Qt-App 是一个基于插件式架构开发的桌面应用程序框架，可用于快速构建功能丰富的跨平台桌面应用。其核心插件系统源自 [Qt Creator](https://github.com/qt-creator/qt-creator/tree/master/src/libs/extensionsystem)，并进行了适当修改。

项目特点：

- 🔌 采用插件化架构，支持功能模块的动态加载与管理
- 📦 使用 [vcpkg](https://github.com/microsoft/vcpkg) 进行依赖管理
- 🛠️ 同时支持 [CMake](.github/workflows/cmake.yml) 和 [QMake](.github/workflows/qmake.yml) 构建系统
- 🍎 支持 Apple Silicon 原生编译
- ⚡ 集成 GitHub Actions 自动化编译、打包和发布流程
- 🚨 内置崩溃报告系统 (CrashReport)

## 项目预览

### 主应用程序

<div align="center">
<img src="docs/Qt-App.jpg" width="90%" height="90%">
</div>

### 崩溃报告程序

<div align="center">
<img src="docs/CrashReport.jpg" width="50%" height="50%">
</div>

## 编译与使用

### 使用 [CMake](.github/workflows/cmake.yml) 构建

```bash
# 配置项目
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake

# 编译项目
cmake --build build

# 生成翻译文件
cmake --build build --target Qt-App_lupdate
```

### 使用 [QMake](.github/workflows/qmake.yml) 构建

```bash
# 生成 Makefile
qmake Qt-App.pro

# 编译项目
make -j$(nproc)
```

## 插件开发

Qt-App 使用基于 Qt Creator 的插件系统，开发者可以轻松创建自己的功能插件：

1. 在 `src/plugins/` 目录下创建新插件文件夹
2. 实现 `IPlugin` 接口
3. 添加插件元数据文件 (`*.json`)
4. 在 `src/plugins/CMakeLists.txt` 或 `src/plugins/plugins.pro` 中注册插件

## 国际化

项目支持多语言国际化，翻译文件位于 `translations/` 目录：

- 生成翻译文件：`cmake --build build --target Qt-App_lupdate`
- 发布翻译文件：`cmake --build build --target Qt-App_lrelease`

> 注意：当前更改翻译设置后需要重启程序才能生效

## 打包与分发

项目提供了多平台打包支持：

- **Windows**  
  使用 `packaging/windows/` 目录下的 Inno Setup 脚本构建安装程序。具体操作可参考 [.github/workflows/cmake.yml](.github/workflows/cmake.yml) 中的相关流程。

- **macOS**  
  通过 `packaging/macos/` 中的脚本可生成 DMG 镜像或 PKG 安装包。构建方法详见 [.github/workflows/cmake.yml](.github/workflows/cmake.yml)。

- **Ubuntu/Debian**  
  - **官方 DEB 包**：基于 `packaging/debian/` 配置，使用 `dpkg-buildpackage` 进行构建，参考 [.github/workflows/debian.yml](.github/workflows/debian.yml)。
  - **快速打包**：使用 `packaging/ubuntu/` 配置配合 `dpkg -b` 快速生成 DEB 包，详见 [.github/workflows/cmake.yml](.github/workflows/cmake.yml)。  
  两种方式均会同步生成对应的 AppImage 文件。

## 注意事项与常见问题

### 平台特定问题

1. **Ubuntu Wayland**: 需要设置环境变量解决窗口移动问题：

   ```cpp
   qputenv("QT_QPA_PLATFORM", "xcb");
   ```

2. **macOS Bundle**: CMake 生成的应用程序包可能需要手动处理 `PkgInfo` 文件

3. **macOS 代码签名**: 打包后的应用程序可能需要 ad-hoc 签名才能正常运行，避免出现 `EXC_BAD_ACCESS (SIGKILL (Code Signature Invalid))` 错误：

   ```bash
   # 对应用程序进行 ad-hoc 签名
   codesign --force --deep --sign - /path/to/Qt-App.app
   
   # 或者对 DMG 包中的应用程序签名
   codesign --force --deep --sign - /Volumes/Qt-App/Qt-App.app
   ```

4. **Unix 系统**: 建议使用静态链接避免动态库依赖问题，或正确设置 RPATH

### 依赖管理

1. **vcpkg 限制**: 目前 vcpkg 单独支持 x64-osx 和 arm64-osx 架构，需要明确指定：
   - CMake: `-DCMAKE_OSX_ARCHITECTURES=x86_64` 或 `-DCMAKE_OSX_ARCHITECTURES=arm64`
   - QMake: `QMAKE_APPLE_DEVICE_ARCHS=x86_64` 或 `QMAKE_APPLE_DEVICE_ARCHS=arm64`

2. **Crashpad 权限**: 在 Unix 系统下需要确保 `crashpad_handler` 有执行权限：

   ```bash
   chmod +x crashpad_handler
   ```

## 目录结构详解

```
Qt-App/
├── cmake/               # CMake 实用函数封装
├── docs/                # 文档和图片资源
├── examples/            # 示例代码
│   └── i18n/            # 国际化示例
├── packaging/           # 打包和发布配置
│   ├── debian/          # Debian 官方打包（dpkg-buildpackage）
│   ├── macos/           # DMG/PKG 安装包制作
│   ├── ubuntu/          # Ubuntu/Debian 快速打包（dpkg -b）
│   └── windows/         # Inno Setup 安装程序制作
├── src/                 # 源代码
│   ├── 3rdparty/        # 第三方库
│   │   ├── qtsingleapplication/  # Qt 单实例应用支持
│   │   └── ui_watchdog/ # UI 看门狗组件
│   ├── aggregation/     # 聚合功能模块
│   ├── apps/            # 应用程序入口
│   │   ├── app/         # 主应用程序
│   │   └── crashreport/ # 崩溃报告程序
│   ├── core/            # 核心基础模块
│   ├── dump/            # 崩溃捕捉功能
│   │   ├── breakpad/    # Google Breakpad 封装
│   │   └── crashpad/    # Google Crashpad 封装
│   ├── extensionsystem/ # 插件系统（源自 Qt Creator）
│   ├── plugins/         # 功能插件
│   │   ├── aboutplugin/     # 关于插件
│   │   ├── coreplugin/      # 核心插件（主界面、菜单等）
│   │   ├── guiplugin/       # GUI 组件插件
│   │   ├── hashplugin/      # 哈希算法插件
│   │   ├── helloplugin/     # Hello 测试插件
│   │   └── systeminfoplugin/# 系统信息插件
│   ├── resource/        # 资源文件（图标、样式表等）
│   ├── solutions/       # 解决方案组件
│   │   ├── spinner/     # 加载指示器
│   │   ├── tasking/     # 任务处理
│   │   └── terminal/    # 终端模拟
│   ├── utils/           # 工具函数库
│   └── widgets/         # 自定义界面组件
├── tests/               # 测试代码
├── translations/        # 国际化翻译文件
└── 配置文件等
```
