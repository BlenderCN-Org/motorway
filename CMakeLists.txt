cmake_minimum_required( VERSION 3.5 FATAL_ERROR )
cmake_policy( VERSION 3.5 )

project( ProjectNya )

set( NYA_BASE_FOLDER "${CMAKE_SOURCE_DIR}" )
set( NYA_BIN_FOLDER "${NYA_BASE_FOLDER}/bin" )
set( NYA_LIB_FOLDER "${NYA_BASE_FOLDER}/lib" )

# Set global debug prefix
# Note that for some reason, CMake applies this prefix to libraries only
set( CMAKE_DEBUG_POSTFIX "_Debug" )
set( CMAKE_CXX_STANDARD 14 )

# Set output paths
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${NYA_LIB_FOLDER} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${NYA_LIB_FOLDER} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${NYA_LIB_FOLDER} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${NYA_LIB_FOLDER} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${NYA_LIB_FOLDER} )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${NYA_LIB_FOLDER} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${NYA_LIB_FOLDER} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${NYA_LIB_FOLDER} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${NYA_LIB_FOLDER} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${NYA_LIB_FOLDER} )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${NYA_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${NYA_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${NYA_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${NYA_BIN_FOLDER} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${NYA_BIN_FOLDER} )

# Include CMake Modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${NYA_BASE_FOLDER}/CMakeModules" )
include( ${CMAKE_MODULE_PATH}/PrecompiledHeader.cmake )
include( ${CMAKE_MODULE_PATH}/MSVCFilters.cmake )
include( ${CMAKE_MODULE_PATH}/BuildFileMacro.cmake )

# Build Parameters
set( NYA_GFX_API "NYA_D3D11" CACHE STRING "Graphics Backend" )
set( NYA_DEVBUILD TRUE CACHE BOOL "Build with developper features" )

# Apply Parameters
add_definitions( -D${NYA_GFX_API} )

# Windows specifics
if ( WIN32 )
    add_definitions( -DNYA_WIN )
    add_definitions( -DNYA_XAUDIO2 )
    add_definitions( -D_CRT_SECURE_NO_WARNINGS ) 
    add_definitions( -DUNICODE -D_UNICODE ) 
    add_definitions( -DNYA_UNICODE )
elseif( UNIX )
    add_definitions( -DNYA_UNIX )
    add_definitions( -DNYA_OPENAL )
    add_definitions( -D_REENTRANT -D__STRICT_ANSI__ )

    find_package( Threads REQUIRED )
    find_package( X11 REQUIRED )
    find_package( OpenAL REQUIRED )
endif( WIN32 )

if ( MSVC )
    add_definitions( -DNYA_MSVC )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions( -DNYA_CLANG )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_definitions( -DNYA_GCC )
endif ( MSVC )

if ( NYA_DEVBUILD )
    add_definitions( -DNYA_DEVBUILD )
endif ( NYA_DEVBUILD )

if ( "${NYA_GFX_API}" MATCHES "NYA_VULKAN" )
    include_directories( ${Vulkan_INCLUDE_DIR} )
endif( "${NYA_GFX_API}" MATCHES "NYA_VULKAN" )

# Add shared include directories (add paths here only if needed)
include_directories( "${NYA_BASE_FOLDER}/Nya" )
include_directories( "${NYA_BASE_FOLDER}/ThirdParty/" )

link_directories( "${NYA_BASE_FOLDER}/lib" )

# Add stuff to build below
add_subdirectory( Nya )
add_subdirectory( NyaEd )
