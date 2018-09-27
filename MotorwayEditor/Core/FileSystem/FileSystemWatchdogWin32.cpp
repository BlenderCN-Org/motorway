/*
    Project Motorway Source Code
    Copyright (C) 2018 Prévost Baptiste

    This file is part of Project Motorway source code.

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
#if FLAN_WIN
#include "FileSystemWatchdogWin32.h"

#include <Core/TaskManager.h>
#include <Core/Environment.h>
#include <Core/StringHelpers.h>

#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/ShaderStageManager.h>

#include <AppShared.h>

FileSystemWatchdog::FileSystemWatchdog()
    : watchdogHandle( nullptr )
    , shutdownSignal( false )
{

}

FileSystemWatchdog::~FileSystemWatchdog()
{
    // Send signal to the watchdog handle first (should work)
    CancelIoEx( watchdogHandle, nullptr );
    CloseHandle( watchdogHandle );
    watchdogHandle = nullptr;

    // Then toggle the atomic boolean (which should properly stop the thread)
    shutdownSignal.store( true );
}

void FileSystemWatchdog::Create()
{
    fnString_t workingDirectory;
    flan::core::RetrieveWorkingDirectory( workingDirectory );
    watchdogHandle = ::CreateFile( workingDirectory.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL );

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
    const DWORD MAX_BUFFER = 1024;

    BYTE Buffer[MAX_BUFFER] = {};
    WCHAR fileNameModified[FILENAME_MAX] = {};

    DWORD dwBytesReturned = 0;
    BOOL changesResult = FALSE;

    while ( 1 ) {
        if ( shutdownSignal.load() ) {
            break;
        }
            
        if ( changesResult && dwBytesReturned != 0 ) {
            const FILE_NOTIFY_INFORMATION* pNotifyInfo = ( FILE_NOTIFY_INFORMATION* )Buffer;
            memcpy( fileNameModified, pNotifyInfo->FileName, pNotifyInfo->FileNameLength );

            if ( pNotifyInfo->Action == FILE_ACTION_MODIFIED || pNotifyInfo->Action == FILE_ACTION_REMOVED ) {
                fnString_t relativeModifiedFilename = fileNameModified;

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

        memset( Buffer, 0, MAX_BUFFER * sizeof( BYTE ) );
        memset( fileNameModified, 0, FILENAME_MAX * sizeof( WCHAR ) );

        changesResult = ReadDirectoryChangesW( watchdogHandle, Buffer, MAX_BUFFER, TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE, &dwBytesReturned, 0, 0 );
    }
}
#endif
#endif
