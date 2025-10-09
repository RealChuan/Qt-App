include(../../../qmake/PlatformLibraries.pri)

QT += widgets network core5compat

DEFINES += COREPLUGIN_LIBRARY
TARGET = $$add_plugin_library(coreplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(widgets) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

HEADERS += \
    configwidget.h \
    coreplugin.hpp \
    coreplugintr.h \
    icore.h \
    mainwindow.h \
    plugindialog.h

SOURCES += \
    configwidget.cpp \
    coreplugin.cpp \
    icore.cpp \
    mainwindow.cpp \
    plugindialog.cpp

DISTFILES += \
    coreplugin.json
    