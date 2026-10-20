set(PROJECT_SOURCES resource_global.hpp resource.cc resource.hpp)

qt_add_resources(RESOURCE_SOURCES resource.qrc)
add_platform_library(resource ${PROJECT_SOURCES} ${RESOURCE_SOURCES})
target_link_libraries(resource PUBLIC Qt::Widgets)
