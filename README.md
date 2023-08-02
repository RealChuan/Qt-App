# Qt-App

- [简体中文](README.md)
- [English](README.en.md)

图片资源等来自于互联网。
本代码仓库，仅供学习，若被他人用于商业用途 与本人无关！ 请遵守许可证！

## Qt-App

基于插件是开发的桌面应用程序框架，可以用于快速开发桌面应用程序；
[qt-creator/src/libs/extensionsystem at master · qt-creator/qt-creator (github.com)](https://github.com/qt-creator/qt-creator/tree/master/src/libs/extensionsystem)；

<div align="center"><img src="doc/Qt-App.jpg" width="90%" height="90%" /></div>

## CrashReport

崩溃报告程序；

<div align="center"><img src="doc/CrashReport.jpg" width="50%" height="50%" /></div>

## 代码结构

1. [3rdparty](3rdparty)：第三方库；
   1. [qtlockedfile](3rdparty/qtlockedfile)：Qt文件锁；
   2. [qtsingleapplication](3rdparty/qtsingleapplication)：Qt单实例；
   3. [breakpad](3rdparty/breakpad.hpp)：基于Google Breakpad封装的崩溃捕捉；
2. [aggregate](aggregate)：聚合；
3. [apps](apps)：应用程序；
   1. [app](apps/app)：Qt-App；
   2. [crashreport](apps/crashreport)：CrashReport；
4. [cmake](cmake)：封装的CMake实用函数；
   1. [utils](cmake/utils.cmake)：实用函数；
5. [core](core)：插件都继承于此；
6. [extensionsystem](extensionsystem)：插件系统，代码来自于Qt-Creator，做了一些修改；
7. [gui](gui)：封装的界面组件；
8. [plugins](plugins)：插件；
   1. [coreplugin](plugins/coreplugin)：核心插件，主界面、菜单、工具栏、状态栏、设置、插件管理器等；
   2. [serialplugin](plugins/serialplugin)：串口插件；
   3. [tcpplugin](plugins/tcpplugin)：TCP插件；
9. [resource](resource)：图片和QSS文件；
10. [utils](utils)：工具函数封装；
