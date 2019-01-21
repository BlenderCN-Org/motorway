file( GLOB_RECURSE SOURCES "*.cpp" "*.c" "*.h" "*.hpp" "*.inl" )

build_file_macros( SOURCES )

add_library( Nya STATIC ${SOURCES} )

# UNIX
target_link_libraries( Nya ${CMAKE_THREAD_LIBS_INIT} )

add_msvc_filters( "${SOURCES}" )

add_precompiled_header( Nya Shared.h )
