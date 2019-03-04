file( GLOB_RECURSE SOURCES "*.cpp" "*.ico" "*.h" "*.rc" "*.manifest" )
file( GLOB_RECURSE SOURCE_MINIZ "${NYA_BASE_FOLDER}/ThirdParty/miniz/miniz.c"
                                "${NYA_BASE_FOLDER}/ThirdParty/miniz/miniz.h" )
                               
build_file_macros( SOURCES )

add_executable( NyaEd ${GUI_TYPE} ${SOURCES} ${SOURCE_MINIZ} )

add_msvc_filters( "${SOURCES}" )

include_directories( "${NYA_BASE_FOLDER}/NyaEd" )

target_link_libraries( NyaEd debug Nya_Debug optimized Nya )

if ( WIN32 )
    target_link_libraries( NyaEd winmm )
    target_link_libraries( NyaEd Pathcch )
    target_link_libraries( NyaEd Shlwapi )
	
    set_target_properties( NyaEd PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS" )
    target_link_libraries( NyaEd xaudio2 )
elseif( UNIX )
    target_link_libraries( NyaEd Flan )
    target_link_libraries( NyaEd xcb xcb-keysyms X11-xcb ${X11_LIBRARIES} )
    target_link_libraries( NyaEd openal )
endif ( WIN32 )

if ( "${NYA_GFX_API}" MATCHES "NYA_GL460" )
    set( OpenGL_GL_PREFERENCE GLVND )
    find_package( OpenGL REQUIRED )
    
    set_target_properties( NyaEd PROPERTIES RELEASE_OUTPUT_NAME "NyaEdGL460" )
    set_target_properties( NyaEd PROPERTIES DEBUG_OUTPUT_NAME "NyaEdGL460_debug" )

    target_link_libraries( NyaEd ${OPENGL_LIBRARIES} )
elseif ( "${NYA_GFX_API}" MATCHES "NYA_D3D11" )
    find_package( DirectX REQUIRED )

    set_target_properties( NyaEd PROPERTIES RELEASE_OUTPUT_NAME "NyaEdD3D11" )
    set_target_properties( NyaEd PROPERTIES DEBUG_OUTPUT_NAME "NyaEdD3D11_debug" )

    target_link_libraries( NyaEd d3d11 )
    target_link_libraries( NyaEd dxgi )
    target_link_libraries( NyaEd dxguid ) # Used by resource labels
elseif ( "${NYA_GFX_API}" MATCHES "NYA_D3D12" )
    find_package( DirectX REQUIRED )

    set_target_properties( NyaEd PROPERTIES RELEASE_OUTPUT_NAME "NyaEdD3D12" )
    set_target_properties( NyaEd PROPERTIES DEBUG_OUTPUT_NAME "NyaEdD3D12_debug" )

    target_link_libraries( NyaEd d3d12 )
    target_link_libraries( NyaEd dxgi )
    target_link_libraries( NyaEd dxguid ) # Used by resource labels
elseif ( "${NYA_GFX_API}" MATCHES "NYA_VULKAN" )
    find_package( Vulkan REQUIRED )
    
    set_target_properties( NyaEd PROPERTIES RELEASE_OUTPUT_NAME "NyaEdVk" )
    set_target_properties( NyaEd PROPERTIES DEBUG_OUTPUT_NAME "NyaEdVk_debug" )

    target_link_libraries( NyaEd Vulkan::Vulkan )
endif( "${NYA_GFX_API}" MATCHES "NYA_GL460" )
