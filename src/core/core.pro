include(../../qmake/PlatformLibraries.pri)

QT += widgets

DEFINES += CORE_LIBRARY
TARGET = $$add_shared_library(core)

HEADERS += \
    core_global.h \
    corewidget.hpp

SOURCES += \
    corewidget.cc
