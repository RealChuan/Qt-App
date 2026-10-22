# cmake-format: off
# 单元测试（QTest + CTest）
#
#   接入：顶层 -DQT_APP_BUILD_TESTS=ON（默认 OFF）。三条打包链
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

if(CMAKE_CONFIGURATION_TYPES)
  # 多配置生成器：_set_target_output_dir 会再追加 /<CONFIG>
  set(UNIT_TEST_OUTPUT_DIR "${PROJECT_OUTPUT_ROOT}/tests")
else()
  string(TOLOWER "${CMAKE_BUILD_TYPE}" _unit_test_config)
  if(NOT _unit_test_config)
    set(_unit_test_config "none")
  endif()
  set(UNIT_TEST_OUTPUT_DIR "${PROJECT_OUTPUT_ROOT}/tests/${_unit_test_config}")
endif()

# cmake-format: off
# 运行时库搜索路径：ctest 直接拉起 exe，不经过启动脚本。
#   - Windows：utils / extensionsystem / shell 是 DLL，且不在测试 exe 旁边
#   - Linux / macOS：Qt 装在自定义前缀，CMake 只把构建树内的目录写进 RPATH
# 目录就地用生成器表达式取，本机与 CI 都不必再手工 export。
# cmake-format: on
if(WIN32)
  set(_unit_test_path_sep ";")
else()
  set(_unit_test_path_sep ":")
endif()

string(
  JOIN
  "${_unit_test_path_sep}"
  _unit_test_lib_dirs
  "$<TARGET_FILE_DIR:Qt6::Core>"
  "$<TARGET_FILE_DIR:utils>"
  "$<TARGET_FILE_DIR:extensionsystem>"
  "$<TARGET_FILE_DIR:shell>")

if(WIN32)
  set(UNIT_TEST_ENV "PATH=${_unit_test_lib_dirs};$ENV{PATH}")
elseif(APPLE)
  set(UNIT_TEST_ENV
      "DYLD_LIBRARY_PATH=${_unit_test_lib_dirs}:$ENV{DYLD_LIBRARY_PATH}")
else()
  set(UNIT_TEST_ENV
      "LD_LIBRARY_PATH=${_unit_test_lib_dirs}:$ENV{LD_LIBRARY_PATH}")
endif()

# add_unit_test(<target> SOURCES <file>... LIBRARIES <target>...)
function(add_unit_test target_name)
  cmake_parse_arguments(ARG "" "" "SOURCES;LIBRARIES" ${ARGN})

  qt_add_executable(${target_name} ${ARG_SOURCES})
  # project_includes 提供统一 include 根 src/，测试因此可以直接 <utils/xxx.hpp>
  target_link_libraries(${target_name} PRIVATE ${ARG_LIBRARIES}
                                               project_includes Qt::Test)
  _set_target_output_dir(${target_name} "${UNIT_TEST_OUTPUT_DIR}")

  add_test(NAME ${target_name}
           COMMAND ${CMAKE_COMMAND} -E env "${UNIT_TEST_ENV}"
                   $<TARGET_FILE:${target_name}>)
  set_tests_properties(${target_name} PROPERTIES TIMEOUT 120)
endfunction()

add_subdirectory(utils)
add_subdirectory(extensionsystem)
add_subdirectory(shell)
