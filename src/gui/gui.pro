include(../slib.pri)

QT += widgets

DEFINES += GUI_LIBRARY
TARGET = $$replaceLibName(gui)

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