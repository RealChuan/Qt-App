# =============================================================================
# 输出目录（与 CMake 的 binaries/<toolchain>/<arch>/... 布局对等）
#
#   binaries/cmake/... ← CMake
#   binaries/qmake/... ← qmake（本文件）
#
# 根目录下按构建系统分家：两条链产物同名（都带 d 后缀），合到一起会互相覆盖。
# =============================================================================

CONFIG(debug, debug|release) {
    BUILD_TYPE_DIR = debug
} else {
    BUILD_TYPE_DIR = release
}

OUTPUT_ROOT = $$PWD/../binaries/qmake/$$ARCH_DIR

RUNTIME_OUTPUT_DIRECTORY = $$OUTPUT_ROOT/bin/$$BUILD_TYPE_DIR
LIBRARY_OUTPUT_DIRECTORY = $$OUTPUT_ROOT/lib/$$BUILD_TYPE_DIR

# 插件落点：与 cmake/InstallConfig.cmake 的 PLUGIN_RUNTIME_SUBDIR 保持一致，
# 否则用其中一条链编出来的目录跑不起来（应用按 applicationDirPath() 找插件）。
macx {
    PLUGIN_RUNTIME_SUBDIR = $${PROJECT_NAME}.app/Contents/PlugIns/$$lower($$PROJECT_NAME)
} else {
    PLUGIN_RUNTIME_SUBDIR = plugins/$$lower($$PROJECT_NAME)
}

!isEmpty(VERBOSE): message("RUNTIME_OUTPUT_DIRECTORY: $$RUNTIME_OUTPUT_DIRECTORY")
!isEmpty(VERBOSE): message("PLUGIN_RUNTIME_SUBDIR: $$PLUGIN_RUNTIME_SUBDIR")

INCLUDEPATH += $$PWD/../src
DEPENDPATH  += $$PWD/../src
