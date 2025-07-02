include(../plugins.pri)
include(../../3rdparty/3rdparty.pri)

QT += widgets network core5compat

DEFINES += GUIPLUGIN_LIBRARY
TARGET = $$replaceLibName(guiplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(widgets) \
    -l$$replaceLibName(utils)

SOURCES += \
    guiplugin.cc \
    guiwidget.cc

HEADERS += \
    guiwidget.hpp

DISTFILES += \
    guiplugin.json
