include(../plugins.pri)

QT += widgets network core5compat

DEFINES += TCPPLUGIN_LIBRARY
TARGET = $$replaceLibName(tcpplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(gui) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

SOURCES += \
    tcpclient.cpp \
    tcpplugin.cc \
    tcpserver.cpp \
    tcpwidget.cpp

HEADERS += \
    tcpclient.h \
    tcpplugin.hpp \
    tcpserver.h \
    tcpwidget.h

DISTFILES += \
    tcpplugin.json
