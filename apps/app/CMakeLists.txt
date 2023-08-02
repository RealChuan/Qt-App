set(PROJECT_SOURCES main.cc)

if(CMAKE_HOST_WIN32)
  qt_add_executable(Qt-App MANUAL_FINALIZATION ${PROJECT_SOURCES} app.rc)
elseif(CMAKE_HOST_APPLE)
  set(CMAKE_MACOSX_BUNDLE ON)
  set(MACOSX_BUNDLE_BUNDLE_NAME "MyApp")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.Youth.\${MACOSX_BUNDLE_BUNDLE_NAME}")
  set(MACOSX_BUNDLE_ICON_FILE "app.icns")
  set(MACOSX_BUNDLE_INFO_STRING "MyApp ${PROJECT_VERSION}")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "${MACOSX_BUNDLE_INFO_STRING}")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING
      "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
  set(MACOSX_BUNDLE_COPYRIGHT "(C) 2023 Youth")
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/app.icns
                              PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  qt_add_executable(Qt-App MACOSX_BUNDLE MANUAL_FINALIZATION ${PROJECT_SOURCES}
                    app.icns)
  add_dependencies(Qt-App coreplugin serialplugin tcpplugin CrashReport)
else()
  qt_add_executable(Qt-App MANUAL_FINALIZATION ${PROJECT_SOURCES})
endif()

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
qt_finalize_executable(Qt-App)

if(CMAKE_HOST_APPLE)
  # copy plugins  to bundle
  add_custom_command(
    TARGET Qt-App
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory ${EXECUTABLE_OUTPUT_PATH}/plugins
      ${EXECUTABLE_OUTPUT_PATH}/Qt-App.app/Contents/MacOS/plugins)
  # copy crashrepoter to bundle
  add_custom_command(
    TARGET Qt-App
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${EXECUTABLE_OUTPUT_PATH}/CrashReport
            ${EXECUTABLE_OUTPUT_PATH}/Qt-App.app/Contents/MacOS/CrashReport)
endif()
