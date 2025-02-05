PROJECT(CommunicationDemo)

file(GLOB_RECURSE PROJECT_ROOT_FILES ${PROJECT_SOURCE_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.c ${PROJECT_SOURCE_DIR}/*.cpp)

set(
    ALL_FILES 
    ${PROJECT_ROOT_FILES}
)

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${ALL_FILES})

ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${ALL_FILES}
)

target_link_libraries(${PROJECT_NAME}
    Log
    Communication
    Time
    global_thread_pool
)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(${PROJECT_NAME}
        pthread
    )
endif()

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/Communication/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/util/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/Log/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/Time/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/global_thread_pool/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/global_timer/include")
