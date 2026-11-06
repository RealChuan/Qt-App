# =============================================================================
# InstallConfig.cmake - 跨平台安装路径配置
#
# 两套路径，其他文件都从这里派生：
#   <X>_RUNTIME_SUBDIR  相对可执行文件目录 —— 构建树
#   <X>_INSTALL_DIR     相对安装前缀       —— 安装树
#
# Linux 的安装前缀是 usr/，与 qt.conf 的 Prefix 对齐；Windows / macOS 是应用根。
# 运行期前缀由 src/utils/appinfo.cc 的 AppInfo::prefix() 解析，两棵树共用一份代码。
# =============================================================================

string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWERCASE)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(APPLE)
    set(CMAKE_INSTALL_PREFIX
        "/Applications"
        CACHE PATH "Install path prefix" FORCE)
  elseif(UNIX)
    set(CMAKE_INSTALL_PREFIX
        "/opt/${PROJECT_NAME}"
        CACHE PATH "Install path prefix" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX
        "C:/Program Files/${PROJECT_NAME}"
        CACHE PATH "Install path prefix" FORCE)
  endif()
endif()

if(APPLE)
  set(PLUGIN_RUNTIME_SUBDIR
      "${PROJECT_NAME}.app/Contents/PlugIns/${PROJECT_NAME_LOWERCASE}")
  set(RESOURCE_RUNTIME_SUBDIR "${PROJECT_NAME}.app/Contents/Resources")
  set(TRANSLATION_RUNTIME_SUBDIR
      "${PROJECT_NAME}.app/Contents/Resources/translations")

  set(MAIN_APP_INSTALL_DIR ".")
  set(TOOL_INSTALL_DIR "${PROJECT_NAME}.app/Contents/MacOS")
  set(LIB_INSTALL_DIR "${PROJECT_NAME}.app/Contents/Frameworks")
  set(PLUGIN_INSTALL_DIR
      "${PROJECT_NAME}.app/Contents/PlugIns/${PROJECT_NAME_LOWERCASE}")
  set(RESOURCE_INSTALL_DIR "${PROJECT_NAME}.app/Contents/Resources")
  set(TRANSLATION_INSTALL_DIR
      "${PROJECT_NAME}.app/Contents/Resources/translations")
elseif(WIN32)
  set(PLUGIN_RUNTIME_SUBDIR "plugins/${PROJECT_NAME_LOWERCASE}")
  set(RESOURCE_RUNTIME_SUBDIR "resources")
  set(TRANSLATION_RUNTIME_SUBDIR "translations")

  set(MAIN_APP_INSTALL_DIR ".")
  set(TOOL_INSTALL_DIR ".")
  set(LIB_INSTALL_DIR ".")
  set(PLUGIN_INSTALL_DIR "plugins/${PROJECT_NAME_LOWERCASE}")
  set(RESOURCE_INSTALL_DIR "resources")
  set(TRANSLATION_INSTALL_DIR "translations")
else()
  set(PLUGIN_RUNTIME_SUBDIR "plugins/${PROJECT_NAME_LOWERCASE}")
  set(RESOURCE_RUNTIME_SUBDIR "resources")
  set(TRANSLATION_RUNTIME_SUBDIR "translations")

  # usr/ 下是 FHS 布局，也是 AppImage 要求的 AppDir 结构
  set(MAIN_APP_INSTALL_DIR "usr/bin")
  set(TOOL_INSTALL_DIR "usr/bin")
  set(LIB_INSTALL_DIR "usr/lib")
  set(PLUGIN_INSTALL_DIR "usr/plugins/${PROJECT_NAME_LOWERCASE}")
  set(RESOURCE_INSTALL_DIR "usr/share/${PROJECT_NAME_LOWERCASE}/resources")
  set(TRANSLATION_INSTALL_DIR "usr/translations")
endif()

set(CMAKE_INSTALL_BINDIR "${MAIN_APP_INSTALL_DIR}")
set(CMAKE_INSTALL_LIBDIR "${LIB_INSTALL_DIR}")

set(INSTALL_CONFIG_LOADED TRUE)
