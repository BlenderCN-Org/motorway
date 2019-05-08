/*
Project Alone Source Code
    Copyright (C) 2018 Prévost Baptiste

    This file is part of Project Alone source code.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <Shared.h>

#if FLAN_DEVBUILD
#if FLAN_UNIX
#include "FileSystemWatchdogUnix.h"

#include <Core/TaskManager.h>
#include <Core/Environment.h>
#include <Core/StringHelpers.h>

#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/ShaderStageManager.h>

#include <AppShared.h>

#include <unistd.h>
#include <sys/inotify.h>

FileSystemWatchdog::FileSystemWatchdog()
    : fsWatchdogHandle( -1 )
    , watchdogHandle( -1 )
    , shutdownSignal( false )
{

}

FileSystemWatchdog::~FileSystemWatchdog()
{
    // Send signal to the watchdog handle first (should work)
    inotify_rm_watch( fsWatchdogHandle, watchdogHandle );
    close( fsWatchdogHandle );

    // Then toggle the atomic boolean (which should properly stop the thread)
    shutdownSignal.store( true );
}

void FileSystemWatchdog::Create()
{
    fnString_t workingDirectory;
    flan::core::RetrieveWorkingDirectory( workingDirectory );

    // Create filesystem watchdog
    fsWatchdogHandle = inotify_init();

    if ( fsWatchdogHandle < 0 ) {
        FLAN_CERR << "Failed to initialize inotify (error code: " << fsWatchdogHandle << ")" << std::endl;
        FLAN_CWARN << "File changes won't be taken in account for this session" << std::endl;
        return;
    }

    // Create directory watchdog
    watchdogHandle = inotify_add_watch( fsWatchdogHandle, workingDirectory.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE );

    if ( watchdogHandle < 0 ) {
        FLAN_CERR << "Failed to initialize watchdog handle (error code: " << watchdogHandle << ")" << std::endl;
        FLAN_CWARN << "File changes won't be taken in account for this session" << std::endl;
        return;
    }

    g_TaskManager->addTask( std::bind( &FileSystemWatchdog::Monitor, this ) );
}

void FileSystemWatchdog::OnFrame()
{
    while ( !materialsToReload.empty() ) {
        auto& materialToReload = materialsToReload.front();
        g_GraphicsAssetManager->getMaterial( materialToReload.c_str(), true );

        materialsToReload.pop();
    }

    while ( !texturesToReload.empty() ) {
        auto& textureToReload = texturesToReload.front();
        g_GraphicsAssetManager->getTexture( textureToReload.c_str(), true );

        texturesToReload.pop();
    }

    while ( !meshesToReload.empty() ) {
        auto& meshToReload = meshesToReload.front();
        g_GraphicsAssetManager->getMesh( meshToReload.c_str(), true );

        meshesToReload.pop();
    }

   /* while ( !modelsToReload.empty() ) {
        auto& modelToReload = modelsToReload.front();
        g_GraphicsAssetManager->getOrLoadModel( modelToReload.c_str(), true );

        modelsToReload.pop();
    }*/
    
    while ( !shadersToReload.empty() ) {
        auto& shaderToReload = shadersToReload.front();
        g_ShaderStageManager->getOrUploadStage( shaderToReload.Filename, shaderToReload.StageType, true );

        shadersToReload.pop();
    }
}

void FileSystemWatchdog::Monitor()
{
    constexpr std::size_t EVENT_SIZE = ( sizeof (struct inotify_event) );
    constexpr std::size_t MAX_BUFFER = ( 1024 * ( EVENT_SIZE + 16 ) );

    char buffer[MAX_BUFFER] = {};

    while ( 1 ) {
        if ( shutdownSignal.load() ) {
            break;
        }

        auto length = read( fsWatchdogHandle, buffer, MAX_BUFFER );

        if ( length < 0 ) {
            break;
        }

        for ( int i = 0; i < length;  ) {
            struct inotify_event* event = reinterpret_cast<struct inotify_event*>( &buffer[ i ] );

            if ( event->len > 0 ) {
                if ( event->mask & IN_DELETE || event->mask & IN_MODIFY ) {
                    FLAN_CLOG << "Changes detected! Trying to reload the asset..." << std::endl;

                    // Ignore directory changes (for now)
                    if ( event->mask & IN_ISDIR ) {
                        continue;
                    }

                    auto relativeModifiedFilename = fnString_t( event->name, event->len );

                    // Get VFS File paths
                    flan::core::RemoveWordFromString( relativeModifiedFilename, FLAN_STRING( "data\\" ) );
                    flan::core::RemoveWordFromString( relativeModifiedFilename, FLAN_STRING( "dev\\" ) );
                    flan::core::RemoveWordFromString( relativeModifiedFilename, FLAN_STRING( "\\" ), FLAN_STRING( "/" ) );

                    auto extension = flan::core::GetFileExtensionFromPath( relativeModifiedFilename );
                    flan::core::StringToLower( extension );

                    if ( !extension.empty() ) {
                        FLAN_CLOG << relativeModifiedFilename << " has been edited" << std::endl;

                        auto vfsPath = FLAN_STRING( "GameData/" ) + fnString_t( relativeModifiedFilename.c_str() );

                        auto extensionHashcode = flan::core::CRC32( extension );
                        switch ( extensionHashcode ) {
                        case FLAN_STRING_HASH( "amat" ):
                        case FLAN_STRING_HASH( "mat" ):
                            materialsToReload.push( vfsPath );
                            break;

                        case FLAN_STRING_HASH( "dds" ):
                        case FLAN_STRING_HASH( "bmp" ):
                        case FLAN_STRING_HASH( "jpeg" ):
                        case FLAN_STRING_HASH( "jpg" ):
                        case FLAN_STRING_HASH( "png" ):
                        case FLAN_STRING_HASH( "tiff" ):
                        case FLAN_STRING_HASH( "gif" ):
                            texturesToReload.push( vfsPath );
                            break;

                        case FLAN_STRING_HASH( "mesh" ):
                            meshesToReload.push( vfsPath );
                            break;

                        case FLAN_STRING_HASH( "mdl" ):
                            modelsToReload.push( vfsPath );
                            break;

                        case FLAN_STRING_HASH( "vso" ):
                        case FLAN_STRING_HASH( ".gl.spvv" ):
                        case FLAN_STRING_HASH( ".vk.spvv" ):
                            flan::core::RemoveWordFromString( vfsPath, FLAN_STRING( "/CompiledShaders/" ) );
                            shadersToReload.push( { vfsPath, SHADER_STAGE_VERTEX } );
                            break;

                        case FLAN_STRING_HASH( "pso" ):
                        case FLAN_STRING_HASH( ".gl.spvp" ):
                        case FLAN_STRING_HASH( ".vk.spvp" ):
                            flan::core::RemoveWordFromString( vfsPath, FLAN_STRING( "/CompiledShaders/" ) );
                            shadersToReload.push( { vfsPath, SHADER_STAGE_PIXEL } );
                            break;

                        case FLAN_STRING_HASH( "cso" ):
                        case FLAN_STRING_HASH( ".gl.spvc" ):
                        case FLAN_STRING_HASH( ".vk.spvc" ):
                            flan::core::RemoveWordFromString( vfsPath, FLAN_STRING( "/CompiledShaders/" ) );
                            shadersToReload.push( { vfsPath, SHADER_STAGE_COMPUTE } );
                            break;
                        }
                    }
                }
            }

            i += EVENT_SIZE + event->len;
        }
    }
}
#endif
#endif
