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
    pluginview.cpp
    pluginview.h)

include_directories(${PROJECT_SOURCE_DIR}/utils)

set_property(SOURCE pluginmanager.cpp PROPERTY SKIP_AUTOMOC ON)

add_share_library(extensionsystem ${PROJECT_SOURCES} ${SOURCE})
target_link_libraries(extensionsystem PRIVATE aggregation utils Qt::Core5Compat
                                              Qt::Widgets tl::expected)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(extensionsystem PRIVATE "EXTENSIONSYSTEM_LIBRARY")
else()
  target_compile_definitions(extensionsystem
                             PRIVATE "EXTENSIONSYSTEM_STATIC_LIBRARY")
endif()
