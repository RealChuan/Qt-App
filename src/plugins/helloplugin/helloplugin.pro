include(../plugins.pri)

QT += widgets

DEFINES += HELLOPLUGIN_LIBRARY
TARGET = $$replaceLibName(helloplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem)

SOURCES += \
    helloplugin.cc \
    hellowidget.cc

HEADERS += \
    helloplugin.hpp \
    hellowidget.hpp

DISTFILES += \
    helloplugin.json
