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

include(../../../qmake/VcpkgDeps.pri)

HEADERS += \
    configwidget.h \
    coreconstants.h \
    coreplugin.hpp \
    coreplugintr.h \
    icore.h \
    mainwindow.h \
    plugindialog.h \
    themechooser.h

SOURCES += \
    configwidget.cpp \
    coreplugin.cpp \
    icore.cpp \
    mainwindow.cpp \
    plugindialog.cpp \
    themechooser.cpp

DISTFILES += \
    coreplugin.json
    