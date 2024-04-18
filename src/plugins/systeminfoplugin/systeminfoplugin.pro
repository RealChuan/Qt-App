include(../plugins.pri)

QT += widgets

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
    systeminfoplugin.hpp \
    systeminfowidget.hpp

DISTFILES += \
    systeminfoplugin.json
