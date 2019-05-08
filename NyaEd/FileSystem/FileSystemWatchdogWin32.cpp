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

#if NYA_DEVBUILD
#if NYA_WIN
#include "FileSystemWatchdogWin32.h"

#include <Core/Environment.h>
#include <Core/StringHelpers.h>

#include <Graphics/GraphicsAssetCache.h>
#include <Graphics/ShaderCache.h>

#include <thread>

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

void FileSystemWatchdog::create()
{
    nyaString_t workingDirectory;
    nya::core::RetrieveWorkingDirectory( workingDirectory );
    watchdogHandle = ::CreateFile( workingDirectory.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL );

    monitorThread = std::thread( std::bind( &FileSystemWatchdog::monitor, this ) );
}

void FileSystemWatchdog::onFrame( GraphicsAssetCache* graphicsAssetManager, ShaderCache* shaderCache )
{
    while ( !materialsToReload.empty() ) {
        auto& materialToReload = materialsToReload.front();
        graphicsAssetManager->getMaterial( materialToReload.c_str(), true );

        materialsToReload.pop();
    }

    while ( !texturesToReload.empty() ) {
        auto& textureToReload = texturesToReload.front();
        graphicsAssetManager->getTexture( textureToReload.c_str(), true );

        texturesToReload.pop();
    }

    while ( !meshesToReload.empty() ) {
        auto& meshToReload = meshesToReload.front();
        graphicsAssetManager->getMesh( meshToReload.c_str(), true );

        meshesToReload.pop();
    }

    while ( !shadersToReload.empty() ) {
        auto& shaderToReload = shadersToReload.front();
        shaderCache->getOrUploadStage( shaderToReload.Filename, shaderToReload.StageType, true );

        shadersToReload.pop();
    }
}

void FileSystemWatchdog::monitor()
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
                nyaString_t relativeModifiedFilename = fileNameModified;

                // Get VFS File paths
                nya::core::RemoveWordFromString( relativeModifiedFilename, NYA_STRING( "data\\" ) );
                nya::core::RemoveWordFromString( relativeModifiedFilename, NYA_STRING( "dev\\" ) );
                nya::core::RemoveWordFromString( relativeModifiedFilename, NYA_STRING( "\\" ), NYA_STRING( "/" ) );

                auto extension = nya::core::GetFileExtensionFromPath( relativeModifiedFilename );
                nya::core::StringToLower( extension );

                if ( !extension.empty() ) {
                    NYA_CLOG << relativeModifiedFilename << " has been edited" << std::endl;

                    auto vfsPath = NYA_STRING( "GameData/" ) + nyaString_t( relativeModifiedFilename.c_str() );

                    auto extensionHashcode = nya::core::CRC32( extension );
                    switch ( extensionHashcode ) {
                    case NYA_STRING_HASH( "amat" ):
                    case NYA_STRING_HASH( "mat" ):
                        materialsToReload.push( vfsPath );
                        break;

                    case NYA_STRING_HASH( "dds" ):
                    case NYA_STRING_HASH( "bmp" ):
                    case NYA_STRING_HASH( "jpeg" ):
                    case NYA_STRING_HASH( "jpg" ):
                    case NYA_STRING_HASH( "png" ):
                    case NYA_STRING_HASH( "tiff" ):
                    case NYA_STRING_HASH( "gif" ):
                        texturesToReload.push( vfsPath );
                        break;

                    case NYA_STRING_HASH( "mesh" ):
                        meshesToReload.push( vfsPath );
                        break;

                    case NYA_STRING_HASH( "vso" ):
                    case NYA_STRING_HASH( ".gl.spvv" ):
                    case NYA_STRING_HASH( ".vk.spvv" ):
                        nya::core::RemoveWordFromString( vfsPath, NYA_STRING( "/CompiledShaders/" ) );
                        shadersToReload.push( { nya::core::WideStringToString( vfsPath ), SHADER_STAGE_VERTEX } );
                        break;

                    case NYA_STRING_HASH( "pso" ):
                    case NYA_STRING_HASH( ".gl.spvp" ):
                    case NYA_STRING_HASH( ".vk.spvp" ):
                        nya::core::RemoveWordFromString( vfsPath, NYA_STRING( "/CompiledShaders/" ) );
                        shadersToReload.push( { nya::core::WideStringToString( vfsPath ), SHADER_STAGE_PIXEL } );
                        break;

                    case NYA_STRING_HASH( "cso" ):
                    case NYA_STRING_HASH( ".gl.spvc" ):
                    case NYA_STRING_HASH( ".vk.spvc" ):
                        nya::core::RemoveWordFromString( vfsPath, NYA_STRING( "/CompiledShaders/" ) );
                        shadersToReload.push( { nya::core::WideStringToString( vfsPath ), SHADER_STAGE_COMPUTE } );
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
