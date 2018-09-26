file( GLOB_RECURSE SOURCES "*.cpp" "*.ico" "*.h" "*.rc" "*.manifest" )

build_file_macros( SOURCES )

add_executable( MotorwayServer ${GUI_TYPE} ${SOURCES} )

add_msvc_filters( "${SOURCES}" )

include_directories( "${FLAN_BASE_FOLDER}/MotorwayServer" )

target_link_libraries( MotorwayServer 
                        debug "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Debug/BulletDynamics_Debug.lib" 
                        optimized "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Release/BulletDynamics.lib" ) 
                        
target_link_libraries( MotorwayServer 
                        debug "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Debug/BulletCollision_Debug.lib" 
                        optimized "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Release/BulletCollision.lib" ) 
                        
target_link_libraries( MotorwayServer 
                        debug "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Debug/LinearMath_Debug.lib" 
                        optimized "${FLAN_BASE_FOLDER}/ThirdParty/bullet3/lib/Release/LinearMath.lib" )
		
include_directories( "${FLAN_BASE_FOLDER}/ThirdParty/enet/include" )
target_link_libraries( MotorwayServer 
                        debug "${FLAN_BASE_FOLDER}/ThirdParty/enet/Debug/enet.lib"
                        optimized "${FLAN_BASE_FOLDER}/ThirdParty/enet/Release/enet.lib" )
                        
if ( WIN32 )
    target_link_libraries( MotorwayServer debug Flan_Debug optimized Flan )
	
	find_library(WSOCK32_LIBRARY wsock32)
    find_library(WS2_32_LIBRARY ws2_32)

    target_link_libraries( MotorwayServer wsock32 ws2_32 )
    target_link_libraries( MotorwayServer winmm )
    target_link_libraries( MotorwayServer Pathcch )
    target_link_libraries( MotorwayServer Shlwapi )
endif ( WIN32 )
