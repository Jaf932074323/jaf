PROJECT(Log)

file(GLOB_RECURSE PROJECT_ROOT_FILES ${PROJECT_SOURCE_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.c ${PROJECT_SOURCE_DIR}/*.cpp)

set(
    ALL_FILES 
    ${PROJECT_ROOT_FILES}
)

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${ALL_FILES})

add_definitions(-D_API_LOG_EXPORT)

add_library(
    ${PROJECT_NAME} SHARED
    ${ALL_FILES}
)

INCLUDE_DIRECTORIES("./")
INCLUDE_DIRECTORIES("../")
INCLUDE_DIRECTORIES("../../include")
INCLUDE_DIRECTORIES("../../3rdparty")
