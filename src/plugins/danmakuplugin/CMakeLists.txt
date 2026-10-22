set(PROJECT_SOURCES
    danmakuplugin.hpp
    danmakuplugin.cc
    danmakuoverlay.hpp
    danmakuoverlay.cc
    danmakuservice.hpp
    danmakuservice.cc)

add_app_plugin(danmakuplugin ${PROJECT_SOURCES})
target_link_libraries(danmakuplugin PRIVATE shell extensionsystem utils Qt::Widgets)
