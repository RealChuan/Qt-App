set(PROJECT_SOURCES
    fractalplugin.hpp
    fractalplugin.cc
    fractalwidget.hpp
    fractalwidget.cc)

add_app_plugin(fractalplugin ${PROJECT_SOURCES})
target_link_libraries(fractalplugin PRIVATE shell extensionsystem utils Qt::Widgets
                                          Qt::Concurrent)
