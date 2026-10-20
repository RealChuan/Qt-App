set(PROJECT_SOURCES
    extensionsystem_global.h
    extensionsystemtr.h
    iplugin.hpp
    iplugin.cc
    plugincontext.hpp
    plugincontext.cc
    pluginmanager.hpp
    pluginmanager.cc
    pluginspec.hpp
    pluginspec.cc)

add_shared_library(extensionsystem ${PROJECT_SOURCES})
# 运行时不碰界面：utils 只是实现细节，Qt 侧只需要 Core。
target_link_libraries(extensionsystem PRIVATE utils PUBLIC Qt::Core)
