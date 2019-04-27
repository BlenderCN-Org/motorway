file( GLOB_RECURSE SOURCES "*.cpp" "*.ico" "*.h" "*.rc" "*.manifest" )
file( GLOB_RECURSE SOURCE_MINIZ "${NYA_BASE_FOLDER}/ThirdParty/miniz/miniz.c"
                                "${NYA_BASE_FOLDER}/ThirdParty/miniz/miniz.h" )
file( GLOB_RECURSE SOURCE_IMGUI "${NYA_BASE_FOLDER}/ThirdParty/imgui/imgui.cpp"
                                "${NYA_BASE_FOLDER}/ThirdParty/imgui/imgui.h"
                                "${NYA_BASE_FOLDER}/ThirdParty/imgui/imconfig.h"
								"${NYA_BASE_FOLDER}/ThirdParty/imgui/imgui_draw.cpp"
								"${NYA_BASE_FOLDER}/ThirdParty/imgui/imgui_widgets.cpp"
								"${NYA_BASE_FOLDER}/ThirdParty/imgui/imstb_rectpack.h"
								"${NYA_BASE_FOLDER}/ThirdParty/imgui/imstb_textedit.h"
								"${NYA_BASE_FOLDER}/ThirdParty/imgui/imstb_truetype.h"
								"${NYA_BASE_FOLDER}/ThirdParty/imgui/imgui_internal.h"
                                "${NYA_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_dx11.cpp"
                                "${NYA_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_dx11.h"
                                "${NYA_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_win32.cpp"
                                "${NYA_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_win32.h" )
                               
build_file_macros( SOURCES )

add_executable( NyaEd ${GUI_TYPE} ${SOURCES} ${SOURCE_MINIZ} ${SOURCE_IMGUI} )

add_msvc_filters( "${SOURCES}" )

include_directories( "${NYA_BASE_FOLDER}/ThirdParty/imgui" )
include_directories( "${NYA_BASE_FOLDER}/NyaEd" )

target_link_libraries( NyaEd debug Nya_Debug optimized Nya )

if ( WIN32 )
    target_link_libraries( NyaEd winmm )
    target_link_libraries( NyaEd Pathcch )
    target_link_libraries( NyaEd Shlwapi )
	
    set_target_properties( NyaEd PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS" )
    target_link_libraries( NyaEd xaudio2 )
elseif( UNIX )
    target_link_libraries( NyaEd Nya )
    target_link_libraries( NyaEd xcb xcb-keysyms X11-xcb ${X11_LIBRARIES} )
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

    include_directories( ${Vulkan_INCLUDE_DIR} )

    target_link_libraries( NyaEd Vulkan::Vulkan )
endif( "${NYA_GFX_API}" MATCHES "NYA_GL460" )
