PROJECT(Communication)

file(GLOB_RECURSE PROJECT_ROOT_FILES ${PROJECT_SOURCE_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.c ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.ipp)

set(
    ALL_FILES 
    ${PROJECT_ROOT_FILES}
)

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${ALL_FILES})

if ("${STR_SYSTEM}" STREQUAL "windows")
    add_definitions(-D_API_COMMUNICATION_EXPORT)
endif()

add_library(
    ${PROJECT_NAME} STATIC
    ${ALL_FILES}
)

target_link_libraries(${PROJECT_NAME}
    Log
    global_timer
)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include/Communication")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/util/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/Log/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/Time/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/global_timer/include")
