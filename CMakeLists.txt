cmake_minimum_required( VERSION 3.5 FATAL_ERROR )
cmake_policy( VERSION 3.5 )

# FLAN GAME ENGINE
project( ProjectMotorway )

set( FLAN_BASE_FOLDER "${CMAKE_SOURCE_DIR}" )
set( FLAN_BIN_FOLDER "${FLAN_BASE_FOLDER}/bin" )
set( FLAN_LIB_FOLDER "${FLAN_BASE_FOLDER}/lib" )

# Set global debug prefix
# Note that for some reason, CMake applies this prefix to libraries only
set( CMAKE_DEBUG_POSTFIX "_Debug" )
set( CMAKE_CXX_STANDARD 14 )

# Set output paths
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${FLAN_LIB_FOLDER} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${FLAN_LIB_FOLDER} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${FLAN_LIB_FOLDER} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${FLAN_LIB_FOLDER} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${FLAN_LIB_FOLDER} )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${FLAN_LIB_FOLDER} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${FLAN_LIB_FOLDER} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${FLAN_LIB_FOLDER} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${FLAN_LIB_FOLDER} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${FLAN_LIB_FOLDER} )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${FLAN_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${FLAN_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${FLAN_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${FLAN_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${FLAN_BIN_FOLDER} )

# Include CMake Modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${FLAN_BASE_FOLDER}/CMakeModules" )
include( ${CMAKE_MODULE_PATH}/PrecompiledHeader.cmake )
include( ${CMAKE_MODULE_PATH}/MSVCFilters.cmake )
include( ${CMAKE_MODULE_PATH}/BuildFileMacro.cmake )

# Build Parameters
set( FLAN_GFX_API "FLAN_D3D11" CACHE STRING "Graphics Backend" )
set( FLAN_DEVBUILD TRUE CACHE BOOL "Build with developper features" )

# Apply Parameters
add_definitions( -D${FLAN_GFX_API} )

add_definitions( -DFLAN_OPENAL )

# Windows specifics
if ( WIN32 )
    add_definitions( -DFLAN_WIN )
    add_definitions( -D_CRT_SECURE_NO_WARNINGS ) 
    add_definitions( -DUNICODE -D_UNICODE ) 
    add_definitions( -DFLAN_UNICODE )
elseif( UNIX )
    add_definitions( -DFLAN_UNIX )
    add_definitions( -D_REENTRANT -D__STRICT_ANSI__ )

    find_package( Threads REQUIRED )
    find_package( X11 REQUIRED )
    find_package( OpenAL REQUIRED )
endif( WIN32 )

# Visual Studio specifics
if ( MSVC )
    add_definitions( -DFLAN_MSVC )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions( -DFLAN_CLANG )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_definitions( -DFLAN_GCC )
endif ( MSVC )

if ( FLAN_DEVBUILD )
    add_definitions( -DFLAN_DEVBUILD )
endif ( FLAN_DEVBUILD )

if ( "${FLAN_GFX_API}" MATCHES "FLAN_VULKAN" )
    include_directories( ${Vulkan_INCLUDE_DIR} )
endif( "${FLAN_GFX_API}" MATCHES "FLAN_VULKAN" )

# Add shared include directories (add paths here only if needed)
include_directories( "${FLAN_BASE_FOLDER}/Flan" )

include_directories( "${FLAN_BASE_FOLDER}/ThirdParty/" )
include_directories( "${FLAN_BASE_FOLDER}/ThirdParty/glm" )
include_directories( "${FLAN_BASE_FOLDER}/ThirdParty/OpenAL" )
include_directories( "${FLAN_BASE_FOLDER}/ThirdParty/enet" )

link_directories( "${FLAN_BASE_FOLDER}/lib" )

# Add stuff to build below
add_subdirectory( Flan )
add_subdirectory( FlanTests )
add_subdirectory( MotorwayClient )
add_subdirectory( MotorwayEditor )
add_subdirectory( MotorwayServer )
