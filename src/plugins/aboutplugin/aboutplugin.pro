include(../../../qmake/PlatformLibraries.pri)

QT += widgets network core5compat

DEFINES += ABOUTPLUGIN_LIBRARY
TARGET = $$add_plugin_library(aboutplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    aboutplugin.cc \
    aboutwidget.cc

HEADERS += \
    aboutwidget.hpp

DISTFILES += \
    aboutplugin.json
