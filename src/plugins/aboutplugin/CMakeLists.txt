set(PROJECT_SOURCES aboutplugin.cc aboutwidget.cc aboutwidget.hpp)

add_custom_plugin(aboutplugin ${PROJECT_SOURCES})
target_link_libraries(aboutplugin PRIVATE core extensionsystem utils
                                          Qt::Widgets tl::expected)
