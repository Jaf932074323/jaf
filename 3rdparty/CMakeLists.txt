set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:Debug>:/MDd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MD> #--|
        $<$<CONFIG:RelWithDebInfo>:/MD> #--|
    )
endif()


ADD_SUBDIRECTORY(googletest)
