set(PROJECT_SOURCES resource_global.hpp resource.cc resource.hpp)

qt_add_resources(RESOURCE_SOURCES resource.qrc)
add_platform_library(resource ${PROJECT_SOURCES} ${RESOURCE_SOURCES})
target_link_libraries(resource PRIVATE Qt::Widgets)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(resource PRIVATE "RESOURCE_LIBRARY")
endif()

set(THEMES_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/resource/themes")
set(THEMES_DEST_DIR "")

if(CMAKE_HOST_APPLE)
  set(THEMES_DEST_DIR
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources/themes"
  )
else()
  set(THEMES_DEST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/themes")
endif()

if(NOT EXISTS "${THEMES_SOURCE_DIR}")
  message(WARNING "Themes source directory not found: ${THEMES_SOURCE_DIR}")
  return()
endif()

add_custom_command(
  TARGET resource
  POST_BUILD
  COMMENT "Deploying themes content to: ${THEMES_DEST_DIR}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${THEMES_DEST_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${THEMES_SOURCE_DIR}"
          "${THEMES_DEST_DIR}"
  VERBATIM)

message(STATUS "Resource library configuration:")
message(STATUS "  - Themes source: ${THEMES_SOURCE_DIR}")
message(STATUS "  - Themes destination: ${THEMES_DEST_DIR}")
