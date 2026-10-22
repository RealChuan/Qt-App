# -----------------------------------------------------------------------------
# 统一 include 根：用 INTERFACE 目标承载，替代顶层的 include_directories(src)
#   link_libraries() 是目录级命令，对本目录及所有子目录后续创建的目标生效，
#   因此无需在每个子项目里重复声明（也与 qmake 的 INCLUDEPATH += src 对齐）。
# -----------------------------------------------------------------------------
add_library(project_includes INTERFACE)
target_include_directories(
  project_includes INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)
link_libraries(project_includes)

add_subdirectory(utils)
add_subdirectory(crashpadclient)
add_subdirectory(resource)
add_subdirectory(widgets)
add_subdirectory(3rdparty)
add_subdirectory(extensionsystem)
add_subdirectory(shell)
add_subdirectory(plugins)
add_subdirectory(apps)
