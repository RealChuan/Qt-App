set(PROJECT_SOURCES crashdialog.cc crashdialog.hpp main.cc)

if(WIN32)
  list(APPEND PROJECT_SOURCES app.rc)
endif()

if(APPLE)
  qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE MANUAL_FINALIZATION
                    ${PROJECT_SOURCES} app.icns PkgInfo)
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
               MACOSX_BUNDLE_EXECUTABLE_NAME "${PROJECT_NAME}"
               MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
               MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
               MACOSX_BUNDLE_COPYRIGHT "(C) 2026 Youth"
               MACOSX_BUNDLE_GUI_IDENTIFIER "com.${PROJECT_NAME}.app"
               MACOSX_BUNDLE_ICON_FILE "app.icns"
               MACOSX_BUNDLE_INFO_STRING "APPL???")
  set_source_files_properties(app.icns PROPERTIES MACOSX_PACKAGE_LOCATION
                                                  Resources)
  set_source_files_properties(PkgInfo PROPERTIES MACOSX_PACKAGE_LOCATION .)
else()
  qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE
                                                 WIN32_EXECUTABLE TRUE)
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE crashpadclient
          extensionsystem
          shell
          thirdparty
          widgets
          resource
          utils
          Qt::Network
          Qt::Widgets
          crashpad::crashpad)

qt_finalize_executable(${PROJECT_NAME})

# 用 $<TARGET_FILE_DIR> 而不是手工拼路径：单/多配置、.app bundle 都自动正确
set(CRASHPAD_TARGET_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

# 构建后增量同步 crashpad_handler（替代 configure 期的一次性拷贝）
setup_crashpad_handler(${PROJECT_NAME} "${CRASHPAD_TARGET_DIR}")

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${MAIN_APP_INSTALL_DIR}
  BUNDLE DESTINATION ${MAIN_APP_INSTALL_DIR})

install_crashpad_handler(${TOOL_INSTALL_DIR})
