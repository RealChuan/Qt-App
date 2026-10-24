include(../../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_app_plugin(tetrisplugin)

LIBS += \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    tetrisplugin.cc \
    tetrispage.cc \
    tetrisboard.cc \
    tetrispreview.cc \
    tetrisgame.cc

HEADERS += \
    tetrisplugin.hpp \
    tetrispage.hpp \
    tetrisboard.hpp \
    tetrispreview.hpp \
    tetriscolors.hpp \
    tetrisgame.hpp

DISTFILES += \
    tetrisplugin.json
