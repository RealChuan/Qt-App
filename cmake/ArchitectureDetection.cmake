# =============================================================================
# Target Architecture Detection
#
# 对外统一输出：x86_64 / arm64 / x86 / universal / unknown
# （目录名 x64 由 OutputDirectories.cmake 负责映射，这里不做改名）
#
# 优先级：
#   1. 显式 -DTARGET_ARCH=...
#   2. CMAKE_OSX_ARCHITECTURES（macOS 通用二进制）
#   3. CMAKE_GENERATOR_PLATFORM（VS 的 -A 参数）
#   4. CMAKE_SYSTEM_PROCESSOR（交叉编译）
#   5. CMAKE_HOST_SYSTEM_PROCESSOR（本机）
# =============================================================================

# 把各种写法归一成内部命名，无法识别时返回空串
function(qtapp_normalize_arch value out_var)
  string(TOLOWER "${value}" _raw)
  if(_raw MATCHES "^(x64|x86_64|amd64)$")
    set(_result "x86_64")
  elseif(_raw MATCHES "^(arm64|aarch64)$")
    set(_result "arm64")
  elseif(_raw MATCHES "^(x86|i[3-6]86|win32)$")
    set(_result "x86")
  elseif(_raw STREQUAL "universal")
    set(_result "universal")
  else()
    set(_result "")
  endif()
  set(${out_var}
      "${_result}"
      PARENT_SCOPE)
endfunction()

# 在没有显式值的情况下推断目标架构
function(qtapp_detect_arch out_var)
  set(_result "")

  # macOS 通用二进制 / 指定架构
  if(CMAKE_OSX_ARCHITECTURES)
    list(FIND CMAKE_OSX_ARCHITECTURES "arm64" _has_arm64)
    list(FIND CMAKE_OSX_ARCHITECTURES "x86_64" _has_x86_64)
    if(_has_arm64 GREATER_EQUAL 0 AND _has_x86_64 GREATER_EQUAL 0)
      set(_result "universal")
    elseif(_has_arm64 GREATER_EQUAL 0)
      set(_result "arm64")
    elseif(_has_x86_64 GREATER_EQUAL 0)
      set(_result "x86_64")
    endif()
  endif()

  # VS 生成器的平台参数
  if(NOT _result AND CMAKE_GENERATOR_PLATFORM)
    qtapp_normalize_arch("${CMAKE_GENERATOR_PLATFORM}" _result)
  endif()

  # 交叉编译时 CMAKE_SYSTEM_PROCESSOR 才代表目标
  if(NOT _result)
    qtapp_normalize_arch("${CMAKE_SYSTEM_PROCESSOR}" _result)
  endif()

  if(NOT _result)
    qtapp_normalize_arch("${CMAKE_HOST_SYSTEM_PROCESSOR}" _result)
  endif()

  if(NOT _result)
    message(
      WARNING
        "无法可靠推断目标架构，请显式传入 -DTARGET_ARCH=<x86_64|arm64|x86|universal>"
    )
    set(_result "unknown")
  endif()

  set(${out_var}
      "${_result}"
      PARENT_SCOPE)
endfunction()

if(TARGET_ARCH)
  qtapp_normalize_arch("${TARGET_ARCH}" _arch_normalized)
  if(NOT _arch_normalized)
    message(
      FATAL_ERROR
        "无法识别的 TARGET_ARCH='${TARGET_ARCH}'，可用值：x86_64 / arm64 / x86 / universal"
    )
  endif()
  set(TARGET_ARCH
      "${_arch_normalized}"
      CACHE STRING "Target architecture (x86_64/arm64/x86/universal)" FORCE)
  message(STATUS "Target architecture (explicit): ${TARGET_ARCH}")
else()
  qtapp_detect_arch(_arch_detected)
  set(TARGET_ARCH
      "${_arch_detected}"
      CACHE STRING "Target architecture (x86_64/arm64/x86/universal)" FORCE)
  message(STATUS "Target architecture (detected): ${TARGET_ARCH}")
endif()
