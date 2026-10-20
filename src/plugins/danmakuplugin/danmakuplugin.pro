include(../../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_app_plugin(danmakuplugin)

LIBS += \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    danmakuplugin.cc \
    danmakuoverlay.cc \
    danmakuservice.cc

HEADERS += \
    danmakuplugin.hpp \
    danmakuoverlay.hpp \
    danmakuservice.hpp

DISTFILES += \
    danmakuplugin.json
