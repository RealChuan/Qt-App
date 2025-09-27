set(PROJECT_SOURCES spinner.cpp spinner.h spinner_global.h)
qt_add_resources(RESOURCE_SOURCES spinner.qrc)

add_platform_library(spinner ${PROJECT_SOURCES} ${RESOURCE_SOURCES})
target_link_libraries(spinner PRIVATE Qt::Widgets)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(spinner PRIVATE "SPINNER_LIBRARY")
else()
  target_compile_definitions(spinner PRIVATE "SPINNER_STATIC_LIBRARY")
endif()
