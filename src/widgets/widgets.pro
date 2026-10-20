include(../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_platform_library(widgets)

LIBS += \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

HEADERS += \
    dialog.hpp \
    mainwidget.hpp \
    messagebox.h \
    buttoniconstatemanager.hpp \
    waitwidget.h \
    widgets_global.hpp 

SOURCES += \
    dialog.cc \
    mainwidget.cc \
    messagebox.cpp \
    buttoniconstatemanager.cc \
    waitwidget.cpp