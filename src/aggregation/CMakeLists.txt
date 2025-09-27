set(PROJECT_SOURCES aggregate.cpp aggregate.h aggregation_global.h)
add_platform_library(aggregation ${PROJECT_SOURCES})
target_link_libraries(aggregation PRIVATE Qt::Core)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(aggregation PRIVATE "AGGREGATION_LIBRARY")
endif()
