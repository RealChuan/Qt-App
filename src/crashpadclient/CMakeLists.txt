set(PROJECT_SOURCES crashpad.hpp crashpad.cc crashpadclient_global.hpp)

add_platform_library(crashpadclient ${PROJECT_SOURCES})
target_link_libraries(crashpadclient PUBLIC utils Qt::Widgets crashpad::crashpad)
