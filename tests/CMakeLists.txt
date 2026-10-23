# cmake-format: off
# 单元测试（QTest + CTest）—— 项目接线
#
#   机制在 cmake/UnitTests.cmake 与 cmake/QtUnitTests.cmake，本文件只放项目接线。
#
#   接入：顶层 -DBUILD_TESTS=ON（默认 OFF）。三条打包链
#   （cmake.yml / debian.yml / 各平台 package 脚本）共用同一份 configure，
#   默认关闭保证它们完全不构建、也不感知测试目标。
#
#   产物隔离：binaries/cmake/<arch>/tests/<config>/
#     packaging/debian/package.sh 会把 binaries/cmake/*/bin/*/ 整个搬进 .deb，
#     所以测试可执行文件绝不能落在 bin/ 下；tests/ 不在任何打包脚本的 glob 里。
#     测试目标也一律不写 install()，走 cmake --install 的那几条链看不到它们。
#
#   命名：目录取被测单元名，目标统一 tst_ 前缀（Qt 生态惯例）。
#   这是「目标名 = 目录名」规则的唯一例外 —— 测试不是可部署目标。
# cmake-format: on

set(UNIT_TEST_OUTPUT_DIR "${PROJECT_OUTPUT_ROOT}/tests")
if(PROJECT_CONFIG_DIR)
  string(APPEND UNIT_TEST_OUTPUT_DIR "/${PROJECT_CONFIG_DIR}")
endif()

# 项目统一的测试入口：固定产物目录与 include 根，其余透传。 project_includes 是 src/ 的 include 根，测试因此可以直接
# <utils/xxx.hpp>。
function(add_app_test target_name)
  qt_add_unit_test(${target_name} OUTPUT_DIR "${UNIT_TEST_OUTPUT_DIR}" INCLUDES
                   project_includes ${ARGN})
endfunction()

add_subdirectory(utils)
add_subdirectory(extensionsystem)
add_subdirectory(shell)
