set(PROJECT_SOURCES main.cc)

if(CMAKE_HOST_WIN32)
  qt_add_executable(Qt-App MANUAL_FINALIZATION ${PROJECT_SOURCES} app.rc)
elseif(CMAKE_HOST_APPLE)
  qt_add_executable(Qt-App MACOSX_BUNDLE MANUAL_FINALIZATION ${PROJECT_SOURCES}
                    app.icns PkgInfo)
  set_target_properties(
    Qt-App
    PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Qt-App"
               MACOSX_BUNDLE_EXECUTABLE_NAME "Qt-App"
               MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
               MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
               MACOSX_BUNDLE_COPYRIGHT "(C) 2023 Youth"
               MACOSX_BUNDLE_GUI_IDENTIFIER "com.Youth.Qt-App"
               MACOSX_BUNDLE_ICON_FILE app.icns
               MACOSX_BUNDLE_INFO_STRING "APPL???")
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/app.icns
                              PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/PkgInfo
                              PROPERTIES MACOSX_PACKAGE_LOCATION .)
  add_dependencies(Qt-App coreplugin serialplugin tcpplugin CrashReport)
  add_rpath(Qt-App)
else()
  qt_add_executable(Qt-App MANUAL_FINALIZATION ${PROJECT_SOURCES})
  add_rpath(Qt-App)
endif()

set_target_properties(Qt-App PROPERTIES WIN32_EXECUTABLE TRUE MACOSX_BUNDLE
                                                              TRUE)

target_link_libraries(
  Qt-App
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

# qt translations file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/translations)
# set(TS_FILES ${PROJECT_SOURCE_DIR}/translations/Qt-App_zh_CN.ts
# ${PROJECT_SOURCE_DIR}/translations/Qt-App_en.ts)
# qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES} OPTIONS
# -no-obsolete) target_sources(Qt-App PRIVATE ${QM_FILES})
# add_custom_target(translations DEPENDS ${QM_FILES}) add_custom_command( TARGET
# Qt-App POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory
# ${EXECUTABLE_OUTPUT_PATH}/translations/ COMMAND ${CMAKE_COMMAND} -E
# copy_if_different ${QM_FILES} ${EXECUTABLE_OUTPUT_PATH}/translations/ COMMENT
# "Copying translations to build dir")

qt_finalize_executable(Qt-App)

if(CMAKE_HOST_APPLE)
  add_custom_command(
    TARGET Qt-App
    POST_BUILD
    # copy plugins  to bundle
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory ${EXECUTABLE_OUTPUT_PATH}/plugins
      ${EXECUTABLE_OUTPUT_PATH}/Qt-App.app/Contents/MacOS/plugins
      # copy crashrepoter to bundle
    COMMAND ${CMAKE_COMMAND} -E copy ${EXECUTABLE_OUTPUT_PATH}/CrashReport
            ${EXECUTABLE_OUTPUT_PATH}/Qt-App.app/Contents/MacOS/CrashReport)
endif()
