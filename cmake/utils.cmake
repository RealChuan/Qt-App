function(set_runtime_to_archive_dir target_name output_dir)
  set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                                  "${output_dir}")

  foreach(OUTPUTCONFIG IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER ${OUTPUTCONFIG} UPPER_CONFIG)
    set_target_properties(
      ${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${UPPER_CONFIG}
                                "${output_dir}")
  endforeach()
endfunction()

function(add_custom_library target_name)
  if(CMAKE_HOST_WIN32)
    add_library(${target_name} SHARED ${ARGN})
    set_runtime_to_archive_dir(${target_name} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    add_custom_command(
      TARGET ${target_name}
      POST_BUILD
      COMMAND
        ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${target_name}>
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_NAME:${target_name}>)
  else()
    add_library(${target_name} ${ARGN})
  endif()
endfunction(add_custom_library)

function(add_custom_plugin target_name)
  add_library(${target_name} SHARED ${ARGN})
  if(CMAKE_HOST_WIN32)
    set_runtime_to_archive_dir(${target_name}
                               "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
  endif()
  add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${target_name}>
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/$<TARGET_FILE_NAME:${target_name}>
  )
endfunction(add_custom_plugin)

function(add_share_library target_name)
  add_library(${target_name} SHARED ${ARGN})
  if(CMAKE_HOST_WIN32)
    set_runtime_to_archive_dir(${target_name}
                               "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
  endif()
  add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${target_name}>
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_NAME:${target_name}>
  )
endfunction(add_share_library)
