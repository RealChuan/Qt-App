include(../../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_app_plugin(coreplugin)

LIBS += \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(widgets) \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    aboutpage.cc \
    commandpalette.cc \
    configpage.cc \
    coreplugin.cc \
    mainwindow.cc \
    pluginpage.cc

HEADERS += \
    aboutpage.hpp \
    commandpalette.hpp \
    configpage.hpp \
    coreplugin.hpp \
    mainwindow.hpp \
    pluginpage.hpp

DISTFILES += \
    coreplugin.json
