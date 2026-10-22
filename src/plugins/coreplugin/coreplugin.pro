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
    appearancepage.cc \
    commandpalette.cc \
    coreplugin.cc \
    mainwindow.cc \
    plugindetailsview.cc \
    pluginpage.cc \
    pluginview.cc

HEADERS += \
    aboutpage.hpp \
    appearancepage.hpp \
    commandpalette.hpp \
    coreplugin.hpp \
    mainwindow.hpp \
    plugindetailsview.hpp \
    pluginpage.hpp \
    pluginview.hpp

DISTFILES += \
    coreplugin.json
