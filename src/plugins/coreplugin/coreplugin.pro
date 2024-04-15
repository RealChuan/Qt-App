include(../plugins.pri)

QT += widgets

DEFINES += COREPLUGIN_LIBRARY
TARGET = $$replaceLibName(coreplugin)

LIBS += \
    -l$$replaceLibName(core) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(gui) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

SOURCES += \
    configwidget.cpp \
    coreplugin.cpp \
    mainwindow.cpp \
    plugindialog.cpp

HEADERS += \
    configwidget.h \
    coreplugin.h \
    coreplugintr.h \
    mainwindow.h \
    plugindialog.h

OTHER_FILES += coreplugin.json
