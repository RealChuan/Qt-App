set(PROJECT_SOURCES
    appwindow.cc
    appwindow.hpp
    dialog.cc
    dialog.hpp
    messagebox.cc
    messagebox.hpp
    modaloverlay.cc
    modaloverlay.hpp
    buttoniconstatemanager.cc
    buttoniconstatemanager.hpp
    roundedrowtreeview.cc
    roundedrowtreeview.hpp
    startupsplash.cc
    startupsplash.hpp
    treemodel.cc
    treemodel.hpp
    widgets_global.hpp)

add_platform_library(widgets ${PROJECT_SOURCES})
target_link_libraries(widgets PUBLIC resource utils Qt::Widgets)
