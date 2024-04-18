include(../plugins.pri)

QT += widgets

DEFINES += GUIPLUGIN_LIBRARY
TARGET = $$replaceLibName(guiplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(gui) \
    -l$$replaceLibName(utils)

SOURCES += \
    guiplugin.cc \
    guiwidget.cc

HEADERS += \
    guiplugin.hpp \
    guiwidget.hpp

DISTFILES += \
    guiplugin.json
