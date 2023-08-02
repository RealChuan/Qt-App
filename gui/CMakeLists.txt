set(PROJECT_SOURCES
    camelcasecursor.cpp
    camelcasecursor.h
    categorysortfiltermodel.cpp
    categorysortfiltermodel.h
    commonwidget.cc
    commonwidget.hpp
    completinglineedit.cpp
    completinglineedit.h
    dialog.cc
    dialog.hpp
    fancylineedit.cpp
    fancylineedit.h
    gui_global.hpp
    historycompleter.cpp
    historycompleter.h
    itemviews.cpp
    itemviews.h
    messbox.cpp
    messbox.h
    multitextcursor.cpp
    multitextcursor.h
    toolbutton.cc
    toolbutton.hpp
    treemodel.cpp
    treemodel.h
    waitwidget.cpp
    waitwidget.h)

add_custom_library(gui ${PROJECT_SOURCES})
target_link_libraries(gui PRIVATE resource utils Qt6::Widgets)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(gui PRIVATE "GUI_LIBRARY")
endif()
