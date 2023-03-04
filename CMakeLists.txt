cmake_minimum_required(VERSION 3.0.0)
project(01 VERSION 0.1.0)

include(CTest)
enable_testing()

set(SOURCES
    main.cpp
    TaskQueue.cpp
    ThreadPool.cpp
)

add_executable(01 ${SOURCES})

target_include_directories(01
    PRIVATE 
        ${PROJECT_SOURCE_DIR}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
