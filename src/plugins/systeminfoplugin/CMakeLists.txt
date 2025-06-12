set(PROJECT_SOURCES systeminfoplugin.cc systeminfowidget.cc
                    systeminfowidget.hpp)

add_custom_plugin(systeminfoplugin ${PROJECT_SOURCES})
target_link_libraries(systeminfoplugin PRIVATE core extensionsystem utils
                                               Qt::Widgets tl::expected)
