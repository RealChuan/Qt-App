# =============================================================================
# macOS Deployment Target Configuration macOS部署目标配置
#
# 本文件在 project() 之前 include（CMAKE_OSX_DEPLOYMENT_TARGET 必须早于编译器
# 探测才生效），那时 APPLE 尚未定义，因此只能用 CMAKE_HOST_APPLE —— 这是本仓库
# 中唯一保留 CMAKE_HOST_* 判断的地方，属于有意的例外。
# =============================================================================

if(CMAKE_HOST_APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  if(DEFINED ENV{MACOSX_DEPLOYMENT_TARGET}
     AND NOT "$ENV{MACOSX_DEPLOYMENT_TARGET}" STREQUAL "")
    set(_deployment_target "$ENV{MACOSX_DEPLOYMENT_TARGET}")
  else()
    set(_deployment_target "14.4")
  endif()

  set(CMAKE_OSX_DEPLOYMENT_TARGET
      "${_deployment_target}"
      CACHE STRING "Minimum OS X deployment version")
  message(
    STATUS "Set macOS deployment target to: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()
