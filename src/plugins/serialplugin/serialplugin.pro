include(../plugins.pri)

QT += widgets serialport core5compat

DEFINES += SERIALPLUGIN_LIBRARY
TARGET = $$replaceLibName(serialplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(gui) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

SOURCES += \
    serialplugin.cc \
    serialport.cpp \
    serialsettings.cc \
    serialwidget.cpp

HEADERS += \
    serialplugin.hpp \
    serialport.h \
    serialsettings.hpp \
    serialwidget.h

DISTFILES += \
    serialplugin.json
