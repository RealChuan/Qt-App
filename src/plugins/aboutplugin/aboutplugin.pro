include(../plugins.pri)

QT += widgets

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
    aboutplugin.hpp \
    aboutwidget.hpp

DISTFILES += \
    aboutplugin.json
