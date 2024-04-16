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
               MACOSX_BUNDLE_COPYRIGHT "(C) 2023 Youth"
               MACOSX_BUNDLE_GUI_IDENTIFIER "com.Youth.${PROJECT_NAME}"
               MACOSX_BUNDLE_ICON_FILE app.icns
               MACOSX_BUNDLE_INFO_STRING "APPL???")
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/app.icns
                              PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/PkgInfo
                              PROPERTIES MACOSX_PACKAGE_LOCATION .)
  add_dependencies(${PROJECT_NAME} coreplugin serialplugin tcpplugin
                   CrashReport)
  add_rpath(${PROJECT_NAME})
else()
  qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES})
  add_rpath(${PROJECT_NAME})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE
                                                 WIN32_EXECUTABLE TRUE)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE extensionsystem
          thirdparty
          gui
          resource
          utils
          Qt6::Network
          Qt6::Core5Compat
          Qt6::Widgets
          unofficial::breakpad::libbreakpad
          unofficial::breakpad::libbreakpad_client)

qt_finalize_executable(${PROJECT_NAME})

if(CMAKE_HOST_APPLE)
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    # copy plugins  to bundle
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory ${EXECUTABLE_OUTPUT_PATH}/plugins
      ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS/plugins
      # copy crashrepoter to bundle
    COMMAND
      ${CMAKE_COMMAND} -E copy ${EXECUTABLE_OUTPUT_PATH}/CrashReport
      ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS/CrashReport)
endif()
