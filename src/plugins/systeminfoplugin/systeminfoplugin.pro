include(../plugins.pri)
include(../../3rdparty/3rdparty.pri)

QT += widgets network core5compat

DEFINES += SYSTEMINFOPLUGIN_LIBRARY
TARGET = $$replaceLibName(systeminfoplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

SOURCES += \
    systeminfoplugin.cc \
    systeminfowidget.cc

HEADERS += \
    systeminfowidget.hpp

DISTFILES += \
    systeminfoplugin.json
