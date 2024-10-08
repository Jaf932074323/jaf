CMAKE_MINIMUM_REQUIRED(VERSION 3.5.0 FATAL_ERROR)

PROJECT(red_black_tree_test)

add_definitions(-DTEST_RED_BLACK_TREE)

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
    gtest
)

INCLUDE_DIRECTORIES("./")
INCLUDE_DIRECTORIES("../")
INCLUDE_DIRECTORIES("../../include")
INCLUDE_DIRECTORIES("../../3rdparty")

IF(MSVC)
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/MP")
    ADD_DEFINITIONS(-D_WIN32_WINNT=0x0600 -D_SCL_SECURE_NO_WARNINGS)
ENDIF(MSVC)