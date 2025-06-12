include(../../slib.pri)

QT += widgets 

DEFINES += SPINNER_LIBRARY
TARGET = $$replaceLibName(spinner)

RESOURCES += \
    spinner.qrc

HEADERS += \
    spinner.h \
    spinner_global.h

SOURCES += \
    spinner.cpp