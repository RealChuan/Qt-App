set(PROJECT_SOURCES main.cc)

if(CMAKE_HOST_WIN32)
  qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES}
                    app.rc)
elseif(CMAKE_HOST_APPLE)
  qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE MANUAL_FINALIZATION
                    ${PROJECT_SOURCES} app.icns PkgInfo)
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
               MACOSX_BUNDLE_EXECUTABLE_NAME "${PROJECT_NAME}"
               MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
               MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
               MACOSX_BUNDLE_COPYRIGHT "(C) 2025 Youth"
               MACOSX_BUNDLE_GUI_IDENTIFIER "com.${PROJECT_NAME}.app"
               MACOSX_BUNDLE_ICON_FILE "app.icns"
               MACOSX_BUNDLE_INFO_STRING "APPL???")
  set_source_files_properties(app.icns PROPERTIES MACOSX_PACKAGE_LOCATION
                                                  Resources)
  set_source_files_properties(PkgInfo PROPERTIES MACOSX_PACKAGE_LOCATION .)
elseif(CMAKE_HOST_LINUX)
  qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE
                                                 WIN32_EXECUTABLE TRUE)
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE dump
          extensionsystem
          thirdparty
          widgets
          resource
          utils
          Qt::Network
          Qt::Core5Compat
          Qt::Widgets
          crashpad::crashpad
          tl::expected)

qt_finalize_executable(${PROJECT_NAME})

if(CMAKE_HOST_APPLE)
  set(CRASHPAD_TARGET_DIR
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/MacOS")
else()
  set(CRASHPAD_TARGET_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

setup_crashpad_handler("${CRASHPAD_TARGET_DIR}")

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${MAIN_APP_INSTALL_DIR}
  BUNDLE DESTINATION ${MAIN_APP_INSTALL_DIR})

get_crashpad_tools_path(crash_handler_path)

install(
  DIRECTORY "${crash_handler_path}/"
  DESTINATION ${TOOL_INSTALL_DIR}
  # 设置权限
  FILE_PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE
  DIRECTORY_PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE)
