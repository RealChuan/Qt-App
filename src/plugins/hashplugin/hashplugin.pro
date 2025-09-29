include(../../../qmake/PlatformLibraries.pri)

QT += widgets network core5compat

DEFINES += HASHPLUGIN_LIBRARY
TARGET = $$add_plugin_library(hashplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(widgets) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgDeps.pri)

HEADERS += \
    cpubenchthread.hpp \
    hashthread.hpp \
    hashwidget.hpp

SOURCES += \
    cpubenchthread.cc \
    hashplugin.cc \
    hashthread.cc \
    hashwidget.cc

DISTFILES += \
    hashplugin.json
