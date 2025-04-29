include(../slib.pri)

QT += widgets

DEFINES += GUI_LIBRARY
TARGET = $$replaceLibName(gui)

LIBS += \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

HEADERS += \
    camelcasecursor.h \
    categorysortfiltermodel.h \
    mainwidget.hpp \
    completinglineedit.h \
    dialog.hpp \
    fancylineedit.h \
    gui_global.hpp \
    historycompleter.h \
    itemviews.h \
    messagebox.h \
    multitextcursor.h \
    pushbutton.hpp \
    treemodel.h \
    waitwidget.h

SOURCES += \
    camelcasecursor.cpp \
    categorysortfiltermodel.cpp \
    mainwidget.cc \
    completinglineedit.cpp \
    dialog.cc \
    fancylineedit.cpp \
    historycompleter.cpp \
    itemviews.cpp \
    messagebox.cpp \
    multitextcursor.cpp \
    pushbutton.cc \
    treemodel.cpp \
    waitwidget.cpp
