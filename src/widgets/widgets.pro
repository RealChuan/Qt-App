include(../../qmake/PlatformLibraries.pri)

QT += widgets

TARGET = $$add_platform_library(widgets)

LIBS += \
    -l$$replaceLibName(resource) \
    -l$$replaceLibName(utils)

HEADERS += \
    appwindow.hpp \
    dialog.hpp \
    messagebox.hpp \
    buttoniconstatemanager.hpp \
    roundedrowtreeview.hpp \
    treemodel.hpp \
    waitwidget.hpp \
    widgets_global.hpp 

SOURCES += \
    appwindow.cc \
    dialog.cc \
    messagebox.cc \
    buttoniconstatemanager.cc \
    roundedrowtreeview.cc \
    treemodel.cc \
    waitwidget.cc