include(../../qmake/PlatformLibraries.pri)

QT += core

DEFINES += AGGREGATION_LIBRARY
TARGET = $$add_platform_library(aggregation)

HEADERS += \
    aggregate.h \
    aggregation_global.h

SOURCES += \
    aggregate.cpp
