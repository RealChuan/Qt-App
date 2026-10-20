# =============================================================================
# Output Directory Configuration
#
#   TARGET_ARCH 只有一个来源：显式 -DTARGET_ARCH 或 ArchitectureDetection.cmake
#   的检测结果（已归一化为 x86_64/arm64/x86/universal）。这里只做「目录映射」。
#
#   产物布局（packaging/ 与 CI 依赖，勿改）：
#     binaries/cmake/<ARCH_DIR>/bin/<config>/
#     binaries/cmake/<ARCH_DIR>/lib/<config>/
#
#   根目录下按构建系统分家（cmake/ 与 qmake/）：两条链的产物同名（都带 d 后缀），
#   合到一个目录会互相覆盖，隔离后也便于确认产物来自哪条链。
# =============================================================================

if(TARGET_ARCH STREQUAL "x86_64")
  set(ARCH_DIR "x64")
elseif(TARGET_ARCH STREQUAL "arm64")
  set(ARCH_DIR "arm64")
elseif(TARGET_ARCH STREQUAL "universal")
  set(ARCH_DIR "universal")
elseif(TARGET_ARCH STREQUAL "x86")
  set(ARCH_DIR "x86")
else()
  set(ARCH_DIR "unknown")
  message(
    WARNING
      "OutputDirectories: 未知 TARGET_ARCH=${TARGET_ARCH}，产物将落到 binaries/unknown/"
  )
endif()

set(QT_APP_OUTPUT_ROOT "${PROJECT_SOURCE_DIR}/binaries/cmake/${ARCH_DIR}")

if(CMAKE_CONFIGURATION_TYPES)
  # 多配置生成器（Visual Studio, Xcode）：每个配置单独目录
  foreach(_config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${_config}" _config_upper)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_config_upper}
        "${QT_APP_OUTPUT_ROOT}/lib/${_config}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_config_upper}
        "${QT_APP_OUTPUT_ROOT}/lib/${_config}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_config_upper}
        "${QT_APP_OUTPUT_ROOT}/bin/${_config}")
  endforeach()

  # 未指定 --config 时的默认目录
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${QT_APP_OUTPUT_ROOT}/lib")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${QT_APP_OUTPUT_ROOT}/lib")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${QT_APP_OUTPUT_ROOT}/bin")

  # 插件等子目录按配置追加 /<CONFIG>（由 _qtapp_set_output_dir 负责拼接）
  set(QT_APP_RUNTIME_BASE "${QT_APP_OUTPUT_ROOT}/bin")
else()
  # 单配置生成器（Makefile, Ninja）
  if(CMAKE_BUILD_TYPE STREQUAL "None" OR NOT CMAKE_BUILD_TYPE)
    set(_output_suffix "none")
  else()
    string(TOLOWER "${CMAKE_BUILD_TYPE}" _output_suffix)
  endif()

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
      "${QT_APP_OUTPUT_ROOT}/lib/${_output_suffix}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
      "${QT_APP_OUTPUT_ROOT}/lib/${_output_suffix}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
      "${QT_APP_OUTPUT_ROOT}/bin/${_output_suffix}")

  # 单配置生成器下 CMAKE_RUNTIME_OUTPUT_DIRECTORY 已含配置后缀
  set(QT_APP_RUNTIME_BASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

file(MAKE_DIRECTORY "${QT_APP_OUTPUT_ROOT}/bin")
file(MAKE_DIRECTORY "${QT_APP_OUTPUT_ROOT}/lib")

message(
  STATUS
    "Output: ${QT_APP_OUTPUT_ROOT}/{bin,lib}/ (ARCH_DIR=${ARCH_DIR}, TARGET_ARCH=${TARGET_ARCH})"
)
