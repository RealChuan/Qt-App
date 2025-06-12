include(../plugins.pri)
include(../../3rdparty/3rdparty.pri)

QT += widgets network core5compat

DEFINES += ABOUTPLUGIN_LIBRARY
TARGET = $$replaceLibName(aboutplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

SOURCES += \
    aboutplugin.cc \
    aboutwidget.cc

HEADERS += \
    aboutwidget.hpp

DISTFILES += \
    aboutplugin.json
