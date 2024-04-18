set(PROJECT_SOURCES systeminfoplugin.cc systeminfoplugin.hpp
                    systeminfowidget.cc systeminfowidget.hpp)

add_custom_plugin(systeminfoplugin ${PROJECT_SOURCES})
target_link_libraries(systeminfoplugin PRIVATE core extensionsystem utils
                                               Qt6::Widgets)
