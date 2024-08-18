CMAKE_MINIMUM_REQUIRED(VERSION 3.5.0 FATAL_ERROR)#cmake最小的要求

PROJECT(Jaf) #项目

# 使用 C++ 20 标准
set(CMAKE_CXX_STANDARD 20)

message("默认编译器路径：${CMAKE_CXX_COMPILER}")
message("编译器:${CMAKE_CXX_COMPILER_ID}")

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(STR_CPU_BIT "x32")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(STR_CPU_BIT "x64")
else()    
    message(FATAL_ERROR "无法判断是32位还是64位")
endif()

if(WIN32)
    set(STR_SYSTEM "windows")
else()
    message(FATAL_ERROR "不支持的操作系统")
endif()

if(WIN32)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")    
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    else()
        message(FATAL_ERROR "不支持的编译器")
    endif()
endif()

#设置编辑的二进制文件输出路径
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/${STR_SYSTEM}/${CMAKE_CXX_COMPILER_ID}/${STR_CPU_BIT}/${CMAKE_BUILD_TYPE}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/${STR_SYSTEM}/${CMAKE_CXX_COMPILER_ID}/${STR_CPU_BIT}/${CMAKE_BUILD_TYPE}")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/${STR_SYSTEM}/${CMAKE_CXX_COMPILER_ID}/${STR_CPU_BIT}/${CMAKE_BUILD_TYPE}")

#编译器选项设置
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #warning C4819: The file ... code page (936).
    #warning C4996: ... use _CRT_SECURE_NO_WARNINGS.
    #warning C4800: forcing value to bool 'true' or 'false'
    #warning C4244 C4267: possible loss of data
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819 /wd4996 /wd4800 /wd4244 /wd4267")
    #fatal error C1128: number of sections exceeded object file format limit: compile with /bigobj
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    #warning LNK4099: PDB '*.pdb' was not found
    #warning LNK4075: ignoring '/EDITANDCONTINUE' due to '/SAFESEH' specification
    #warning LNK4221: This object file does not define any previously undefined public symbols
    #warning LNK4098: defaultlib 'LIBCMTD' conflicts with use of other libs; use /NODEFAULTLIB:library
    #warning LNK4197: export 'XXX' specified multiple times
    SET(IGNORE_WARNING "/IGNORE:4099,4075,4221,4098,4197,4996")
    SET(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    ${IGNORE_WARNING} ")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${IGNORE_WARNING} ")
    SET(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ${IGNORE_WARNING} ")
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${IGNORE_WARNING} ")
ENDIF()

#添加子目录
ADD_SUBDIRECTORY(3rdparty)
ADD_SUBDIRECTORY(src)