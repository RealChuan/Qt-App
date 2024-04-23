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
    commonwidget.hpp \
    completinglineedit.h \
    dialog.hpp \
    fancylineedit.h \
    gui_global.hpp \
    historycompleter.h \
    itemviews.h \
    messbox.h \
    multitextcursor.h \
    pushbutton.hpp \
    treemodel.h \
    waitwidget.h

SOURCES += \
    camelcasecursor.cpp \
    categorysortfiltermodel.cpp \
    commonwidget.cc \
    completinglineedit.cpp \
    dialog.cc \
    fancylineedit.cpp \
    historycompleter.cpp \
    itemviews.cpp \
    messbox.cpp \
    multitextcursor.cpp \
    pushbutton.cc \
    treemodel.cpp \
    waitwidget.cpp
