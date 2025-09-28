set(PROJECT_SOURCES guiplugin.cc guiwidget.cc guiwidget.hpp)

add_platform_plugin(guiplugin ${PROJECT_SOURCES})
target_link_libraries(guiplugin PRIVATE core extensionsystem widgets utils
                                        Qt::Widgets tl::expected)
