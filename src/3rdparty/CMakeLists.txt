set(PROJECT_SOURCES
    qtsingleapplication/qtsingleapplication.cpp
    qtsingleapplication/qtsingleapplication.h
    qtsingleapplication/qtlocalpeer.cpp
    qtsingleapplication/qtlocalpeer.h
    thirdparty_global.hpp)

add_custom_library(thirdparty ${PROJECT_SOURCES})
target_link_libraries(thirdparty PRIVATE Qt::Network Qt::Widgets)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(thirdparty PRIVATE "THRIDPARTY_LIBRARY")
endif()
