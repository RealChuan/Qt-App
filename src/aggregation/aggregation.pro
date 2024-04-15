include(../libs.pri)

DEFINES += AGGREGATION_LIBRARY
TARGET = $$replaceLibName(aggregation)

HEADERS += \
    aggregate.h \
    aggregation_global.h

SOURCES += \
    aggregate.cpp
