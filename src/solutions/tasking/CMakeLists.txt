set(PROJECT_SOURCES
    barrier.cpp
    barrier.h
    concurrentcall.h
    conditional.cpp
    conditional.h
    networkquery.cpp
    networkquery.h
    qprocesstask.cpp
    qprocesstask.h
    tasking_global.h
    tasktree.cpp
    tasktree.h
    tasktreerunner.cpp
    tasktreerunner.h
    tcpsocket.cpp
    tcpsocket.h)

add_custom_library(tasking ${PROJECT_SOURCES})
target_link_libraries(tasking PRIVATE Qt::Concurrent Qt::Network)

if(CMAKE_HOST_WIN32)
  target_compile_definitions(tasking PRIVATE "TASKING_LIBRARY")
else()
  target_compile_definitions(tasking PRIVATE "TASKING_STATIC_LIBRARY")
endif()
