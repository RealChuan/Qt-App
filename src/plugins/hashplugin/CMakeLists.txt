set(PROJECT_SOURCES
    cpubenchthread.cc
    cpubenchthread.hpp
    hashplugin.cc
    hashplugin.hpp
    hashthread.cc
    hashthread.hpp
    hashwidget.cc
    hashwidget.hpp)

add_custom_plugin(hashplugin ${PROJECT_SOURCES})
target_link_libraries(hashplugin PRIVATE core extensionsystem gui resource
                                         utils Qt6::Widgets)
