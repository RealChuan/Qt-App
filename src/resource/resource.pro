include(../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_platform_library(resource)

SOURCES += \
    resource.cc

HEADERS += \
    resource_global.hpp \
    resource.hpp

RESOURCES += \
    resource.qrc
