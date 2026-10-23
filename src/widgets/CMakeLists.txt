set(PROJECT_SOURCES
    appwindow.cc
    appwindow.hpp
    dialog.cc
    dialog.hpp
    messagebox.cc
    messagebox.hpp
    buttoniconstatemanager.cc
    buttoniconstatemanager.hpp
    treemodel.cc
    treemodel.hpp
    waitwidget.cc
    waitwidget.hpp
    widgets_global.hpp)

add_platform_library(widgets ${PROJECT_SOURCES})
target_link_libraries(widgets PUBLIC resource utils Qt::Widgets)
