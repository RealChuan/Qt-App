# Qt-App

- [简体中文](README.md)
- [English](README.en.md)

> 图片资源等来自于互联网。
本代码仓库，仅供学习，若被他人用于商业用途 与本人无关！ 请遵守许可证！

## Qt-App

- 插件式开发的桌面应用程序框架，可以用于快速开发桌面应用程序；
[qt-creator/src/libs/extensionsystem at master · qt-creator/qt-creator (github.com)](https://github.com/qt-creator/qt-creator/tree/master/src/libs/extensionsystem)；
- 配合[vcpkg](https://github.com/microsoft/vcpkg)进行使用；
- 同时支持[cmake](.github/workflows/cmake.yml)和[qmake](.github/workflows/qmake.yml)编译；
- [支持Apple Silicon原生编译；](#问题和备注)
- 支持[actions](.github/workflows/cmake.yml)编译，打包、发布；

<div align="center"><img src="doc/Qt-App.jpg" width="90%" height="90%" /></div>

## CrashReport

崩溃报告程序；

<div align="center"><img src="doc/CrashReport.jpg" width="50%" height="50%" /></div>

## 代码结构

1. [cmake](cmake)：封装的CMake实用函数；
   1. [utils](cmake/utils.cmake)：实用函数；
2. [doc](doc)：文档说明和图片；
3. [examples](examples)：示例代码;
4. [packaging](packaging)：打包和发布；
5. [src](src)：源码；
   1. [3rdparty](src/3rdparty)：第三方库；
      1. [qtlockedfile](src/3rdparty/qtlockedfile)：Qt文件锁；
      2. [qtsingleapplication](src/3rdparty/qtsingleapplication)：Qt单实例；
   2. [aggregate](src/aggregate)：聚合；
   3. [apps](src/apps)：应用程序；
      1. [app](src/apps/app)：Qt-App；
      2. [crashreport](src/apps/crashreport)：CrashReport；
   4. [core](src/core)：插件都继承于此；
   5. [dump](src/dump)：崩溃捕捉功能；
      1. [breakpad](src/dump/breakpad.hpp)：基于Google Breakpad封装的崩溃捕捉；
      2. [crashpad](src/dump/crashpad.hpp)：基于Google Crashpad封装的崩溃捕捉；
         > 在unix系统下，可能需要对`crashpad_handler`赋予执行权限，否则无法正常启动。

            ```bash
            chmod +x crashpad_handler
            ```

   6. [extensionsystem](src/extensionsystem)：插件系统，代码来自于Qt-Creator，做了一些修改；
   7. [gui](src/gui)：封装的界面组件；
   8. [plugins](src/plugins)：插件；
      1. [aboutplugin](src/plugins/aboutplugin)：关于插件;
      2. [coreplugin](src/plugins/coreplugin)：核心插件，主界面、菜单、工具栏、状态栏、设置、插件管理器等；
      3. [guiplugin](src/plugins/guiplugin)：GUI插件，一些基于QSS样式定制的GUI组件；
      4. [hashplugin](src/plugins/hashplugin)：哈希插件，QT提供的哈希算法；
      5. [helloplugin](src/plugins/helloplugin)：Hello插件，用于测试插件开发；
      6. [systeminfoplugin](src/plugins/systeminfoplugin)：系统信息插件;
   9. [resource](resource)：图片和QSS文件；
   10. [utils](utils)：工具函数封装；
6. [translations](translations)：翻译文件；

## 问题和备注

- > MacOS，cmake生成的bundle，在.app/Contents/文件夹下没有生成`PkgInfo`文件；
   1. [app/CMakeLists](/apps/app/CMakeLists.txt)，使用这个CMakeLists.txt可以在MacOS上生成bundle，也可以正常显示图标，但是没有PkgInfo文件；
   2. cmake该怎么生成PkgInfo文件？
      1. WireShark使用`set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/PkgInfo PROPERTIES MACOSX_PACKAGE_LOCATION .)`类似这种方式，拷贝到bundle中；
   3. qmake默认会生成PkgInfo文件，只需要指定`TARGET=app`或者`CONFIG+=bundle`即可；

- > Unix 系统下，需要尽量使用静态库，避免动态库的依赖问题；
   1. 本项目有几个模块是动态库，因为是plugin，需要动态加载；
   2. 然后需要对这几个动态库进行打包，然后在运行时加载，还需要对rpath`"-Wl,-rpath,\'\$$ORIGIN\':\'\$$ORIGIN/lib\':'\$$ORIGIN/../lib'")`，进行设置，否则会找不到动态库；
   3. 或者使用install_name_tool(macos)、patchelf/chrpath(linux)修改动态库的依赖路径，非常麻烦；
   4. 还要考虑到这些库都是可以共享的，所以不要重复打包；
   5. 具体可以看[workflows](.github/workflows/cmake.yml)；

- > MacOS，[vcpkg](https://github.com/microsoft/vcpkg)编译第三方库问题；
   1. 由于[vcpkg](https://github.com/microsoft/vcpkg)目前[只支持单独编译x64-osx和arm64-osx](https://github.com/microsoft/vcpkg/discussions/19454)；
   2. 在使用[cmake](.github/workflows/cmake.yml)时，需要指定`CMAKE_OSX_ARCHITECTURES=x86_64`或者`CMAKE_OSX_ARCHITECTURES=arm64`;
   3. 在使用[qmake](.github/workflows/qmake.yml)时，需要指定`QMAKE_APPLE_DEVICE_ARCHS=x86_64`或者`QMAKE_APPLE_DEVICE_ARCHS=arm64`；

- > 国际化实时翻译，当前更改完翻译设置，需要重启程序才能生效；
   1. 懒得改代码了；
   2. [具体参考：QT实用小技巧（想到就更新）](https://realchuan.github.io/2021/10/12/QT%E5%AE%9E%E7%94%A8%E5%B0%8F%E6%8A%80%E5%B7%A7%EF%BC%88%E6%83%B3%E5%88%B0%E5%B0%B1%E6%9B%B4%E6%96%B0%EF%BC%89/)，核心代码；

      ```cpp
      void Widget::changeEvent(QEvent *e)
      {
         QWidget::changeEvent(e);
         switch (e->type()) {
         case QEvent::LanguageChange:
            comboBox->setItemText(0, tr("Hello"));
            label->setText(tr("Hello")); // 代码添加的文字
            ui->retranslateUi(this);     // 有UI文件情况下
            break;
         default: break;
         }
      }
      ```
