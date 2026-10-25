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
    modaloverlay.hpp \
    buttoniconstatemanager.hpp \
    roundedrowtreeview.hpp \
    startupsplash.hpp \
    treemodel.hpp \
    widgets_global.hpp 

SOURCES += \
    appwindow.cc \
    dialog.cc \
    messagebox.cc \
    modaloverlay.cc \
    buttoniconstatemanager.cc \
    roundedrowtreeview.cc \
    startupsplash.cc \
    treemodel.cc