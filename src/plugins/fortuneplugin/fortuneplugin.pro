include(../../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_app_plugin(fortuneplugin)

LIBS += \
    -l$$replaceLibName(shell) \
    -l$$replaceLibName(extensionsystem) \
    -l$$replaceLibName(widgets) \
    -l$$replaceLibName(utils)

include(../../../qmake/VcpkgToolchain.pri)

SOURCES += \
    fortuneplugin.cc \
    fortunepage.cc \
    fortuneform.cc \
    fortunereadingview.cc \
    fortunesharing.cc \
    hexagramboard.cc \
    fortuneengine.cc \
    fortunelunar.cc \
    fortunecontent.cc

HEADERS += \
    fortuneplugin.hpp \
    fortunepage.hpp \
    fortuneform.hpp \
    fortunereadingview.hpp \
    fortunesharing.hpp \
    fortunestrings.hpp \
    hexagramboard.hpp \
    fortuneengine.hpp \
    fortunelunar.hpp \
    fortunecontent.hpp

DISTFILES += \
    fortuneplugin.json
