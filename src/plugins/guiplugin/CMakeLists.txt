set(PROJECT_SOURCES guiplugin.cc guiplugin.hpp guiwidget.cc guiwidget.hpp)

add_custom_plugin(guiplugin ${PROJECT_SOURCES})
target_link_libraries(guiplugin PRIVATE core extensionsystem gui utils
                                        Qt6::Widgets)
