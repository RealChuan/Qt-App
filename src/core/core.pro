include(../lib.pri)

QT += widgets

DEFINES += CORE_LIBRARY
TARGET = $$replaceLibName(core)

HEADERS += \
    core_global.h \
    corewidget.hpp

SOURCES += \
    corewidget.cc
