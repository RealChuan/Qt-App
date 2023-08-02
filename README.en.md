# Qt-App

-   [Simplified Chinese](README.md)
-   [English](README.en.md)

Picture resources and so on come from the Internet.
This code warehouse is only for learning, if it is used by others for commercial purposes, it has nothing to do with me! Please obey the license!

## Qt-App

A desktop application framework developed based on plug-ins, which can be used to quickly develop desktop applications;[qt-creator/src/libs/extensionsystem at master · qt-creator/qt-creator (github.com)](https://github.com/qt-creator/qt-creator/tree/master/src/libs/extensionsystem)；

<div align="center"><img src="doc/Qt-App.jpg" width="90%" height="90%" /></div>

## CrashReport

Crash Reporter;

<div align="center"><img src="doc/CrashReport.jpg" width="50%" height="50%" /></div>

## code structure

1.  [3rdparty](3rdparty): third-party library;
    1.  [qtlockedfile](3rdparty/qtlockedfile): Qt file lock;
    2.  [qtsingleapplication](3rdparty/qtsingleapplication): Qt single instance;
    3.  [breakpad](3rdparty/breakpad.hpp): Crash capture based on Google Breakpad package;
2.  [aggregate](aggregate):polymerization;
3.  [apps](apps):application;
    1.  [app](apps/app)：Qt-App；
    2.  [crashreport](apps/crashreport)：CrashReport；
4.  [cmake](cmake): Encapsulated CMake utility function;
    1.  [utils](cmake/utils.cmake): utility function;
5.  [core](core): All plugins inherit from this;
6.  [extensionsystem](extensionsystem): Plug-in system, the code comes from Qt-Creator, with some modifications;
7.  [gui](gui): encapsulated interface component;
8.  [plugins](plugins): plugin;
    1.  [coreplugin](plugins/coreplugin): Core plugin, main interface, menu, toolbar, status bar, settings, plugin manager, etc.;
    2.  [serialplugin](plugins/serialplugin): serial port plug-in;
    3.  [tcpplugin](plugins/tcpplugin): TCP plugin;
9.  [resource](resource): pictures and QSS files;
10. [utils](utils): utility function package;
