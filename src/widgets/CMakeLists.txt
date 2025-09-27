set(PROJECT_SOURCES
    mainwidget.cc
    mainwidget.hpp
    dialog.cc
    dialog.hpp
    messagebox.cpp
    messagebox.h
    pushbutton.cc
    pushbutton.hpp
    waitwidget.cpp
    waitwidget.h
    widgets_global.hpp)

add_platform_library(widgets ${PROJECT_SOURCES})
target_link_libraries(widgets PRIVATE resource utils Qt::Widgets)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(widgets PRIVATE "WIDGETS_LIBRARY")
endif()
