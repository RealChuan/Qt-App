include(../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_platform_library(widgets)

LIBS += \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

HEADERS += \
    dialog.hpp \
    mainwidget.hpp \
    messagebox.hpp \
    buttoniconstatemanager.hpp \
    treemodel.hpp \
    waitwidget.hpp \
    widgets_global.hpp 

SOURCES += \
    dialog.cc \
    mainwidget.cc \
    messagebox.cc \
    buttoniconstatemanager.cc \
    treemodel.cc \
    waitwidget.cc