include(../../../qmake/PlatformLibraries.pri)

QT += widgets network core5compat

DEFINES += SYSTEMINFOPLUGIN_LIBRARY
TARGET = $$add_plugin_library(systeminfoplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    systeminfoplugin.cc \
    systeminfowidget.cc

HEADERS += \
    systeminfowidget.hpp

DISTFILES += \
    systeminfoplugin.json
