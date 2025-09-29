include(../../../qmake/PlatformLibraries.pri)

QT += widgets network core5compat

DEFINES += GUIPLUGIN_LIBRARY
TARGET = $$add_plugin_library(guiplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(widgets) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgDeps.pri)

SOURCES += \
    guiplugin.cc \
    guiwidget.cc

HEADERS += \
    guiwidget.hpp

DISTFILES += \
    guiplugin.json
