set(PROJECT_SOURCES
    mainwidget.cc
    mainwidget.hpp
    dialog.cc
    dialog.hpp
    gui_global.hpp
    messagebox.cpp
    messagebox.h
    pushbutton.cc
    pushbutton.hpp
    waitwidget.cpp
    waitwidget.h)

add_custom_library(gui ${PROJECT_SOURCES})
target_link_libraries(gui PRIVATE resource utils Qt::Widgets)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(gui PRIVATE "GUI_LIBRARY")
endif()
