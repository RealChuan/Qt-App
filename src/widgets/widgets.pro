include(../../qmake/PlatformLibraries.pri)

QT += widgets

DEFINES += WIDGETS_LIBRARY
TARGET = $$add_platform_library(widgets)

LIBS += \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

HEADERS += \
    dialog.hpp \
    gui_global.hpp \
    mainwidget.hpp \
    messagebox.h \
    pushbutton.hpp \
    waitwidget.h

SOURCES += \
    dialog.cc \
    mainwidget.cc \
    messagebox.cpp \
    pushbutton.cc \
    waitwidget.cpp