file( GLOB_RECURSE SOURCES "*.cpp" "*.c" "*.h" "*.hpp" "*.inl" )

build_file_macros( SOURCES )

add_library( Flan STATIC ${SOURCES} )

target_link_libraries( Flan ${CMAKE_THREAD_LIBS_INIT} )

add_msvc_filters( "${SOURCES}" )

add_precompiled_header( Flan Shared.h )

include_directories( "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/src" )
include_directories( "${FLAN_BASE_FOLDER}/ThirdParty/enet/include" )
include_directories( "${FLAN_BASE_FOLDER}/ThirdParty/DirectXTex" )