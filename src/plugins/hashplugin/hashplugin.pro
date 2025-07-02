include(../plugins.pri)
include(../../3rdparty/3rdparty.pri)

QT += widgets network core5compat

DEFINES += HASHPLUGIN_LIBRARY
TARGET = $$replaceLibName(hashplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(widgets) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

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
