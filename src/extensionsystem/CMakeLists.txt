set(PROJECT_SOURCES
    extensionsystem_global.hpp
    extensionsystemtr.hpp
    iplugin.hpp
    iplugin.cc
    plugincontext.hpp
    plugincontext.cc
    pluginmanager.hpp
    pluginmanager.cc
    pluginspec.hpp
    pluginspec.cc)

add_shared_library(extensionsystem ${PROJECT_SOURCES})
# iplugin.hpp / plugincontext.hpp 的公开头文件里 include 了 utils/filepath.hpp，
# 所以 utils 必须 PUBLIC：否则使用者得自己再链一次 utils 才能解析这些符号。
target_link_libraries(extensionsystem PUBLIC utils Qt::Core)
