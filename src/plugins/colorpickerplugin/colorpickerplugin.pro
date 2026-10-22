include(../../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_app_plugin(colorpickerplugin)

LIBS += \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    colorpickerplugin.cc \
    colorpickerpage.cc \
    colorpickerwidget.cc

HEADERS += \
    colorpickerplugin.hpp \
    colorpickerpage.hpp \
    colorpickerwidget.hpp

DISTFILES += \
    colorpickerplugin.json
