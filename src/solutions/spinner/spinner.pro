include(../../../qmake/PlatformLibraries.pri)

QT += widgets 

DEFINES += SPINNER_LIBRARY
TARGET = $$add_platform_library(spinner)

RESOURCES += \
    spinner.qrc

HEADERS += \
    spinner.h \
    spinner_global.h

SOURCES += \
    spinner.cpp
