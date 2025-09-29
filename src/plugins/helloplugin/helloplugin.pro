include(../../../qmake/PlatformLibraries.pri)

QT += widgets network core5compat

DEFINES += HELLOPLUGIN_LIBRARY
TARGET = $$add_plugin_library(helloplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgDeps.pri)

SOURCES += \
    helloplugin.cc \
    hellowidget.cc

HEADERS += \
    hellowidget.hpp

DISTFILES += \
    helloplugin.json
