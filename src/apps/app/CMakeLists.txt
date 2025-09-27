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
               MACOSX_BUNDLE_GUI_IDENTIFIER "com.Youth.${PROJECT_NAME}"
               MACOSX_BUNDLE_ICON_FILE "app.icns"
               MACOSX_BUNDLE_INFO_STRING "APPL???")
  set_source_files_properties(app.icns PROPERTIES MACOSX_PACKAGE_LOCATION
                                                  Resources)
  set_source_files_properties(PkgInfo PROPERTIES MACOSX_PACKAGE_LOCATION .)
  add_dependencies(${PROJECT_NAME} CrashReport)
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

string(REPLACE "share/crashpad" "tools/crashpad" crash_handler_path
               "${crashpad_DIR}")
message(STATUS "Crashpad tools directory: ${crash_handler_path}")

if(NOT EXISTS "${crash_handler_path}")
  message(
    FATAL_ERROR "Crashpad tools directory not found: ${crash_handler_path}")
endif()

if(CMAKE_HOST_APPLE)
  set(BUNDLE_CONTENTS_DIR
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/MacOS")

  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMENT "Configuring macOS application bundle with crashpad"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_CONTENTS_DIR}"
    COMMAND
      ${CMAKE_COMMAND} -E copy "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CrashReport"
      "${BUNDLE_CONTENTS_DIR}/CrashReport"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${crash_handler_path}"
            "${BUNDLE_CONTENTS_DIR}"
    COMMAND chmod +x "${BUNDLE_CONTENTS_DIR}/crashpad_handler"
    VERBATIM)
else()
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMENT "Configuring crashpad handler for ${CMAKE_SYSTEM_NAME}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${crash_handler_path}"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    VERBATIM)
  if(CMAKE_HOST_LINUX)
    add_custom_command(
      TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMENT "Setting execute permissions for crashpad_handler on Linux"
      COMMAND chmod +x "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crashpad_handler"
      VERBATIM)
  endif()
endif()
