set(PROJECT_SOURCES crashpad.hpp crashpad.cc dump_global.hpp)

add_platform_library(dump ${PROJECT_SOURCES})
target_link_libraries(dump PUBLIC utils Qt::Widgets crashpad::crashpad)
