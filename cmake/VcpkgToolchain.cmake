# =============================================================================
# Vcpkg Toolchain Configuration
# =============================================================================

option(USE_VCPKG "Use vcpkg for dependency management" ON)

function(configure_vcpkg_toolchain)
  if(NOT USE_VCPKG)
    message(STATUS "Vcpkg is disabled by configuration")
    return()
  endif()

  # 如果已经设置了工具链文件，则不再覆盖
  if(CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Toolchain file already set: ${CMAKE_TOOLCHAIN_FILE}")
    return()
  endif()

  # 平台特定的vcpkg路径
  if(CMAKE_HOST_WIN32)
    set(VCPKG_DEFAULT_PATH "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
  else()
    set(VCPKG_DEFAULT_PATH
        "/usr/local/share/vcpkg/scripts/buildsystems/vcpkg.cmake")
  endif()

  # 设置工具链文件路径
  set(CMAKE_TOOLCHAIN_FILE
      "${VCPKG_DEFAULT_PATH}"
      CACHE STRING "Vcpkg toolchain file")

  # 检查工具链文件是否存在
  if(EXISTS "${VCPKG_DEFAULT_PATH}")
    message(STATUS "Vcpkg toolchain file: ${VCPKG_DEFAULT_PATH}")
  else()
    message(WARNING "Vcpkg toolchain file not found: ${VCPKG_DEFAULT_PATH}")
    message(STATUS "Please install vcpkg or set CMAKE_TOOLCHAIN_FILE manually")
  endif()
endfunction()

configure_vcpkg_toolchain()
