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
#include "FileSystemNative.h"

#include "FileSystemObjectNative.h"

#if NYA_UNIX
#include "FileSystemUnix.h"
#elif NYA_WIN
#include "FileSystemWin32.h"
#endif

#include <Core/Environment.h>

using namespace nya::core;

FileSystemNative::FileSystemNative( const nyaString_t& customWorkingDirectory )
{
    if ( customWorkingDirectory.empty() ) {
        RetrieveWorkingDirectory( workingDirectory );
    } else {
        workingDirectory = customWorkingDirectory;
    }

    if ( !workingDirectory.empty() && workingDirectory.back() != '/' ) {
        workingDirectory += '/';
    }
}

FileSystemNative::~FileSystemNative()
{
    workingDirectory.clear();

    while ( !openedFiles.empty() ) {
        auto& openedFile = openedFiles.back();
        // TODO Fucks up the CRT at closing; probably due to some race condition or poor thread sync...
        //openedFile->close();
        delete openedFile;

        openedFiles.pop_back();
    }
}

FileSystemObject* FileSystemNative::openFile( const nyaString_t& filename, const int32_t mode )
{
    const bool useWriteMode = ( ( mode & eFileOpenMode::FILE_OPEN_MODE_WRITE ) == eFileOpenMode::FILE_OPEN_MODE_WRITE );

    if ( ( !fileExists( filename ) && !useWriteMode ) || ( useWriteMode && isReadOnly() ) ) {
        return nullptr;
    }

    // Build flagset
    int32_t openMode = 0;
    if ( ( mode & eFileOpenMode::FILE_OPEN_MODE_READ ) == eFileOpenMode::FILE_OPEN_MODE_READ ) {
        openMode |= std::ios::in;
    }

    if ( ( mode & eFileOpenMode::FILE_OPEN_MODE_WRITE ) == eFileOpenMode::FILE_OPEN_MODE_WRITE ) {
        openMode |= std::ios::out;
    }

    if ( ( mode & eFileOpenMode::FILE_OPEN_MODE_BINARY ) == eFileOpenMode::FILE_OPEN_MODE_BINARY ) {
        openMode |= std::ios::binary;
    }

    if ( ( mode & eFileOpenMode::FILE_OPEN_MODE_APPEND ) == eFileOpenMode::FILE_OPEN_MODE_APPEND ) {
        openMode |= std::ios::app;
    }

    if ( ( mode & eFileOpenMode::FILE_OPEN_MODE_TRUNCATE ) == eFileOpenMode::FILE_OPEN_MODE_TRUNCATE ) {
        openMode |= std::ios::trunc;
    }

    if ( ( mode & eFileOpenMode::FILE_OPEN_MODE_START_FROM_END ) == eFileOpenMode::FILE_OPEN_MODE_START_FROM_END ) {
        openMode |= std::ios::ate;
    }

    // Check if the file has already been opened
    auto fileHashcode = CRC32( filename );
    for ( auto openedFile : openedFiles ) {
        if ( openedFile->getHashcode() == fileHashcode && !openedFile->isOpen() ) {
            openedFile->open( openMode );
            return openedFile;
        }
    }

    // If the file has never been opened, open it
    openedFiles.push_back( new FileSystemObjectNative( filename ) );

    auto openedFile = openedFiles.back();
    openedFile->open( openMode );

    return openedFile;
}

void FileSystemNative::closeFile( FileSystemObject* fileSystemObject )
{
    if ( fileSystemObject == nullptr ) {
        return;
    }

    openedFiles.remove_if( [=]( FileSystemObject* obj ) { 
        return obj->getHashcode() == fileSystemObject->getHashcode(); 
    } );

    delete fileSystemObject;
}

void FileSystemNative::createFolder( const nyaString_t& folderName )
{
    CreateFolderImpl( folderName );
}

bool FileSystemNative::fileExists( const nyaString_t& filename )
{
    return FileExistsImpl( filename );
}

bool FileSystemNative::isReadOnly()
{
    return false;
}

nyaString_t FileSystemNative::resolveFilename( const nyaString_t& mountPoint, const nyaString_t& filename )
{
    nyaString_t resolvedFilename( filename );
    return resolvedFilename.replace( resolvedFilename.begin(), resolvedFilename.begin() + mountPoint.length(), workingDirectory );
}
