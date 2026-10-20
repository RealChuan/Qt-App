# =============================================================================
# Vcpkg Toolchain Configuration
#
# vcpkg 的安装位置在本项目里是固定的，直接硬编码，不做多候选探测：
#   - VS 安装器的 vcpkg 组件会把 VCPKG_ROOT 环境变量指向自带的空仓库
#     （%ProgramFiles%\Microsoft Visual Studio\<ver>\<edition>\VC\vcpkg），
#     跟随环境变量会走错目录并触发依赖全量重编
#   - 需要换位置时改 VCPKG_DEFAULT_ROOT 或 -DVCPKG_DEFAULT_ROOT=<dir>
#
# 注意：本文件在 project() 之前 include，WIN32/APPLE 尚不可用。
# =============================================================================

option(USE_VCPKG "Use vcpkg for dependency management" ON)

if(NOT USE_VCPKG)
  message(STATUS "vcpkg: 已被 USE_VCPKG=OFF 关闭")
  return()
endif()

# 外部工具链（交叉编译等，非 vcpkg 自带）原样尊重
if(CMAKE_TOOLCHAIN_FILE AND NOT CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg\\.cmake$")
  message(STATUS "vcpkg: 使用外部工具链 ${CMAKE_TOOLCHAIN_FILE}，跳过 vcpkg 配置")
  return()
endif()

# -----------------------------------------------------------------------------
# 根目录（默认值即平台的常规安装位置，CACHE 只为方便在 cmake-gui 里改）
# -----------------------------------------------------------------------------
if(CMAKE_HOST_WIN32)
  set(_default_root "C:/vcpkg")
else()
  set(_default_root "/usr/local/share/vcpkg")
endif()

set(VCPKG_DEFAULT_ROOT
    "${_default_root}"
    CACHE PATH "vcpkg 根目录")

set(_vcpkg_toolchain "${VCPKG_DEFAULT_ROOT}/scripts/buildsystems/vcpkg.cmake")
if(NOT EXISTS "${_vcpkg_toolchain}")
  message(
    WARNING
      "vcpkg: 未找到 ${_vcpkg_toolchain}\n"
      "  请安装 vcpkg，或用 -DVCPKG_DEFAULT_ROOT=<dir> 指定根目录")
  return()
endif()

# 强制写回缓存：本项目只认上面这一个位置，顺便覆盖历史配置里残留的旧路径
set(CMAKE_TOOLCHAIN_FILE
    "${_vcpkg_toolchain}"
    CACHE FILEPATH "Vcpkg toolchain file" FORCE)
message(STATUS "vcpkg: 工具链 ${CMAKE_TOOLCHAIN_FILE}")

# -----------------------------------------------------------------------------
# 默认 triplet 与 qmake 侧保持一致（Windows: 静态 vcpkg 库 + 动态 CRT）
#   目标机器上没装这个 triplet 时保持 vcpkg 默认值，避免静默从源码重编依赖
# -----------------------------------------------------------------------------
set(_triplet_arch "x64")
if(TARGET_ARCH MATCHES "^(arm64|aarch64|ARM64)$")
  set(_triplet_arch "arm64")
elseif(NOT TARGET_ARCH AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
  set(_triplet_arch "arm64")
endif()

if(CMAKE_HOST_WIN32)
  set(_triplet_suffix "windows-static-md")
elseif(CMAKE_HOST_APPLE)
  set(_triplet_suffix "osx")
else()
  set(_triplet_suffix "linux")
endif()

set(_triplet "${_triplet_arch}-${_triplet_suffix}")

if(EXISTS "${VCPKG_DEFAULT_ROOT}/installed/${_triplet}")
  set(VCPKG_TARGET_TRIPLET
      "${_triplet}"
      CACHE STRING "vcpkg target triplet" FORCE)
  message(STATUS "vcpkg: triplet ${_triplet}（与 qmake 侧一致）")
else()
  message(
    STATUS
      "vcpkg: 未安装 ${_triplet}，沿用 vcpkg 默认 triplet；"
      "需与 qmake 侧一致时执行：vcpkg install --triplet ${_triplet}")
endif()
