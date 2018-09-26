file( GLOB_RECURSE SOURCES "*.cpp" "*.ico" "*.h" "*.rc" "*.manifest" )
file( GLOB_RECURSE SOURCES_IMGUI "${FLAN_BASE_FOLDER}/ThirdParty/imgui/imconfig.h"
                                 "${FLAN_BASE_FOLDER}/ThirdParty/imgui/imgui.cpp"
                                 "${FLAN_BASE_FOLDER}/ThirdParty/imgui/imgui.h"
                                 "${FLAN_BASE_FOLDER}/ThirdParty/imgui/imgui_internal.h"
                                 "${FLAN_BASE_FOLDER}/ThirdParty/imgui/imgui_draw.cpp" 
                                 "${FLAN_BASE_FOLDER}/ThirdParty/ImGuizmo/ImGuizmo.cpp"
                                 "${FLAN_BASE_FOLDER}/ThirdParty/ImGuizmo/ImGuizmo.h" )

if ( "${FLAN_GFX_API}" MATCHES "FLAN_D3D11" )
	file( GLOB_RECURSE SOURCES_IMGUI_IMPL "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_dx11.cpp"
										   "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_dx11.h"
										   "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_win32.cpp"
										   "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_win32.h"  )
elseif ( "${FLAN_GFX_API}" MATCHES "FLAN_GL460" )
	file( GLOB_RECURSE SOURCES_IMGUI_IMPL "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_opengl3.cpp"
										   "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_opengl3.h"
										   "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_win32.cpp"
										   "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_win32.h"
										   "${FLAN_BASE_FOLDER}/ThirdParty/GL/gl3w.c"  )
elseif ( "${FLAN_GFX_API}" MATCHES "FLAN_VULKAN" )
	file( GLOB_RECURSE SOURCES_IMGUI_IMPL "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_vulkan.cpp"
										   "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_vulkan.h"
										   "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_win32.cpp"
										   "${FLAN_BASE_FOLDER}/ThirdParty/imgui/examples/imgui_impl_win32.h" )
endif( "${FLAN_GFX_API}" MATCHES "FLAN_D3D11" )

file( GLOB_RECURSE SOURCE_MINIZ "${PH_BASE_FOLDER}/ThirdParty/miniz/miniz.c"
                                "${PH_BASE_FOLDER}/ThirdParty/miniz/miniz.h" )
                               
build_file_macros( SOURCES )

add_executable( MotorwayClient ${GUI_TYPE} ${SOURCES} ${SOURCE_MINIZ} ${SOURCES_IMGUI_IMPL} ${SOURCES_IMGUI} )

add_msvc_filters( "${SOURCES}" )

include_directories( "${FLAN_BASE_FOLDER}/MotorwayClient" )
include_directories( "${FLAN_BASE_FOLDER}/ThirdParty/imgui/" )

target_link_libraries( MotorwayClient 
                        debug "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Debug/BulletDynamics_Debug.lib" 
                        optimized "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Release/BulletDynamics.lib" ) 
                        
target_link_libraries( MotorwayClient 
                        debug "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Debug/BulletCollision_Debug.lib" 
                        optimized "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Release/BulletCollision.lib" ) 
                        
target_link_libraries( MotorwayClient 
                        debug "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Debug/LinearMath_Debug.lib" 
                        optimized "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Release/LinearMath.lib" )
		
include_directories( "${FLAN_BASE_FOLDER}/ThirdParty/enet/include" )
target_link_libraries( MotorwayClient 
                        debug "${FLAN_BASE_FOLDER}/ThirdParty/enet/Debug/enet.lib"
                        optimized "${FLAN_BASE_FOLDER}/ThirdParty/enet/Release/enet.lib" )
                        
if ( WIN32 )
    target_link_libraries( MotorwayClient debug Flan_Debug optimized Flan )

    target_link_libraries( MotorwayClient winmm )
    target_link_libraries( MotorwayClient Pathcch )
    target_link_libraries( MotorwayClient Shlwapi )
    target_link_libraries( MotorwayClient Hid ) # RawInput
	
	find_library(WSOCK32_LIBRARY wsock32)
    find_library(WS2_32_LIBRARY ws2_32)

    target_link_libraries( MotorwayClient wsock32 ws2_32 )

    set_target_properties( MotorwayClient PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS" )
    
    target_link_libraries( MotorwayClient debug "${FLAN_BASE_FOLDER}/ThirdParty/OpenAL/Debug/OpenAL32.lib"
                                     optimized "${FLAN_BASE_FOLDER}/ThirdParty/OpenAL/Release/OpenAL32.lib"        )
    target_link_libraries( MotorwayClient xaudio2 )
elseif( UNIX )
    target_link_libraries( MotorwayClient Flan )
    target_link_libraries( MotorwayClient xcb xcb-keysyms X11-xcb ${X11_LIBRARIES} )
    target_link_libraries( MotorwayClient openal )
endif ( WIN32 )

if ( "${FLAN_GFX_API}" MATCHES "FLAN_GL460" )
    set( OpenGL_GL_PREFERENCE GLVND )
    find_package( OpenGL REQUIRED )
    
    set_target_properties( MotorwayClient PROPERTIES RELEASE_OUTPUT_NAME "MotorwayClientGL460" )
    set_target_properties( MotorwayClient PROPERTIES DEBUG_OUTPUT_NAME "MotorwayClientGL460_debug" )

    target_link_libraries( MotorwayClient ${OPENGL_LIBRARIES} )
elseif ( "${FLAN_GFX_API}" MATCHES "FLAN_D3D11" )
    find_package( DirectX REQUIRED )

    set_target_properties( MotorwayClient PROPERTIES RELEASE_OUTPUT_NAME "MotorwayClientD3D11" )
    set_target_properties( MotorwayClient PROPERTIES DEBUG_OUTPUT_NAME "MotorwayClientD3D11_debug" )

    target_link_libraries( MotorwayClient d3d11 )
    target_link_libraries( MotorwayClient dxgi )
    target_link_libraries( MotorwayClient dxguid ) # Used by resource labels
    target_link_libraries( MotorwayClient d3dcompiler )
    
    target_link_libraries( MotorwayClient 
                            debug "${FLAN_BASE_FOLDER}/ThirdParty/DirectXTex/DirectXTex/Bin/Desktop_2017/x64/Debug/DirectXTex.lib"
                            optimized "${FLAN_BASE_FOLDER}/ThirdParty/DirectXTex/DirectXTex/Bin/Desktop_2017/x64/Release/DirectXTex.lib" )
elseif ( "${FLAN_GFX_API}" MATCHES "FLAN_VULKAN" )
    find_package( Vulkan REQUIRED )
    
    set_target_properties( MotorwayClient PROPERTIES RELEASE_OUTPUT_NAME "MotorwayClientVk" )
    set_target_properties( MotorwayClient PROPERTIES DEBUG_OUTPUT_NAME "MotorwayClientVk_debug" )

    target_link_libraries( MotorwayClient Vulkan::Vulkan )
endif( "${FLAN_GFX_API}" MATCHES "FLAN_GL460" )
