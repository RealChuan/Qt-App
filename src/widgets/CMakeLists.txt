set(PROJECT_SOURCES
    mainwidget.cc
    mainwidget.hpp
    dialog.cc
    dialog.hpp
    messagebox.cpp
    messagebox.h
    buttoniconstatemanager.cc
    buttoniconstatemanager.hpp
    waitwidget.cpp
    waitwidget.h
    widgets_global.hpp)

add_platform_library(widgets ${PROJECT_SOURCES})
target_link_libraries(widgets PUBLIC resource utils Qt::Widgets)
