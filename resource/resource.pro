include(../libs.pri)

QT += widgets

DEFINES += RESOURCE_LIBRARY
TARGET = $$replaceLibName(resource)

SOURCES += \
    resource.cc

HEADERS += \
    resource_global.hpp \
    resource.hpp

RESOURCES += \
    resource.qrc
