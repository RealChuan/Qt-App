set(PROJECT_SOURCES core_global.h corewidget.cc corewidget.hpp)
add_share_library(core ${PROJECT_SOURCES})
target_link_libraries(core PRIVATE Qt::Widgets)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(core PRIVATE "CORE_LIBRARY")
endif()
