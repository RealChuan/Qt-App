set(PROJECT_SOURCES resource_global.hpp resource.cc resource.hpp)
qt_add_resources(RESOURCE_SOURCES resource.qrc)

add_custom_library(resource ${PROJECT_SOURCES} ${RESOURCE_SOURCES})
target_link_libraries(resource PRIVATE Qt::Widgets)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(resource PRIVATE "RESOURCE_LIBRARY")
endif()

if(CMAKE_HOST_APPLE)
  add_custom_command(
    TARGET resource
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory
      "${PROJECT_SOURCE_DIR}/src/resource/themes"
      "${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/Resources/themes"
    COMMENT "Copying themes directory...")
else()
  add_custom_command(
    TARGET resource
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory
      "${PROJECT_SOURCE_DIR}/src/resource/themes"
      "${EXECUTABLE_OUTPUT_PATH}/resources/themes"
    COMMENT "Copying themes directory...")
endif()
