set(PROJECT_SOURCES
    extensionsystem_global.h
    extensionsystemtr.h
    invoker.cpp
    invoker.h
    iplugin.cpp
    iplugin.h
    optionsparser.cpp
    optionsparser.h
    plugindetailsview.cpp
    plugindetailsview.h
    pluginerroroverview.cpp
    pluginerroroverview.h
    pluginerrorview.cpp
    pluginerrorview.h
    pluginmanager.cpp
    pluginmanager.h
    pluginmanager_p.h
    pluginspec.cpp
    pluginspec.h
    pluginspec_p.h
    pluginview.cpp
    pluginview.h)

include_directories(${PROJECT_SOURCE_DIR}/utils)

set_property(SOURCE pluginmanager.cpp PROPERTY SKIP_AUTOMOC ON)

add_share_library(extensionsystem ${PROJECT_SOURCES} ${SOURCE})
target_link_libraries(extensionsystem PRIVATE aggregation gui resource utils
                                              Qt6::Core5Compat Qt6::Widgets)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(extensionsystem PRIVATE "EXTENSIONSYSTEM_LIBRARY")
endif()
