set(PROJECT_SOURCES
    camelcasecursor.cpp
    camelcasecursor.h
    categorysortfiltermodel.cpp
    categorysortfiltermodel.h
    mainwidget.cc
    mainwidget.hpp
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
    messagebox.cpp
    messagebox.h
    multitextcursor.cpp
    multitextcursor.h
    pushbutton.cc
    pushbutton.hpp
    treemodel.cpp
    treemodel.h
    waitwidget.cpp
    waitwidget.h)

add_custom_library(gui ${PROJECT_SOURCES})
target_link_libraries(gui PRIVATE resource utils Qt::Widgets)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(gui PRIVATE "GUI_LIBRARY")
endif()
