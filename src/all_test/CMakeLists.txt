PROJECT(AllTest)

file(GLOB_RECURSE PROJECT_ROOT_FILES ${PROJECT_SOURCE_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.c ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE UtilTest_FILES ${CMAKE_SOURCE_DIR}/src/UtilTest/gtest/*.h ${CMAKE_SOURCE_DIR}/src/UtilTest/gtest/*.c ${CMAKE_SOURCE_DIR}/src/UtilTest/gtest/*.cpp)
file(GLOB_RECURSE red_black_tree_test_FILES ${CMAKE_SOURCE_DIR}/src/red_black_tree_test/gtest/*.h ${CMAKE_SOURCE_DIR}/src/red_black_tree_test/gtest/*.c ${CMAKE_SOURCE_DIR}/src/red_black_tree_test/gtest/*.cpp)
add_definitions(-DTEST_RED_BLACK_TREE)
file(GLOB_RECURSE CommunicationTest_FILES ${CMAKE_SOURCE_DIR}/src/CommunicationTest/gtest/*.h ${CMAKE_SOURCE_DIR}/src/CommunicationTest/gtest/*.c ${CMAKE_SOURCE_DIR}/src/CommunicationTest/gtest/*.cpp)
file(GLOB_RECURSE TimeTest_FILES ${CMAKE_SOURCE_DIR}/src/TimeTest/gtest/*.h ${CMAKE_SOURCE_DIR}/src/TimeTest/gtest/*.c ${CMAKE_SOURCE_DIR}/src/TimeTest/gtest/*.cpp)

set(
    ALL_FILES 
    ${PROJECT_ROOT_FILES}
    ${UtilTest_FILES}
    ${red_black_tree_test_FILES}
    ${CommunicationTest_FILES}
    ${TimeTest_FILES}
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
    global_timer
    gtest
)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/util/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/Communication/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/Log/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/Time/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/global_thread_pool/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/global_timer/include")
