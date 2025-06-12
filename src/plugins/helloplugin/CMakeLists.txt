set(PROJECT_SOURCES helloplugin.cc hellowidget.cc hellowidget.hpp)

add_custom_plugin(helloplugin ${PROJECT_SOURCES})
target_link_libraries(helloplugin PRIVATE core extensionsystem utils
                                          Qt::Widgets tl::expected)
