file( GLOB_RECURSE SOURCES "*.cpp" "*.ico" "*.h" "*.rc" "*.manifest" )

build_file_macros( SOURCES )

add_executable( FlanTests ${SOURCES} )

set_target_properties( FlanTests PROPERTIES RELEASE_OUTPUT_NAME "FlanTests" )
set_target_properties( FlanTests PROPERTIES DEBUG_OUTPUT_NAME "FlanTests_debug" )

add_msvc_filters( "${SOURCES}" )

include_directories( "${FLAN_BASE_FOLDER}/FlanTests" )

target_link_libraries( FlanTests debug Flan_Debug optimized Flan )

if( UNIX )
        target_link_libraries( FlanTests openal )
elseif ( WIN32 )
    target_link_libraries( FlanTests debug "${FLAN_BASE_FOLDER}/ThirdParty/OpenAL/Debug/OpenAL32.lib"
                                     optimized "${FLAN_BASE_FOLDER}/ThirdParty/OpenAL/Release/OpenAL32.lib"        )
    target_link_libraries( FlanTests xaudio2 )
    target_link_libraries( FlanTests Shlwapi )
endif ( UNIX )
