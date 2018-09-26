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
#include "VirtualFileSystem.h"

VirtualFileSystem::VirtualFileSystem()
{

}

VirtualFileSystem::~VirtualFileSystem()
{
    fileSystemEntries.clear();
}

void VirtualFileSystem::mount( FileSystem* media, const fnString_t& mountPoint, const uint64_t mountOrder )
{
    fileSystemEntries.push_back( { mountPoint, media, mountOrder } );

    fileSystemEntries.sort( [=]( FileSystemEntry& lEntry, FileSystemEntry& rEntry ) {
        return lEntry.MountPoint.length() >= rEntry.MountPoint.length() && lEntry.MountOrder <= rEntry.MountOrder;
    } );
}

void VirtualFileSystem::unmount( FileSystem* media )
{
    fileSystemEntries.remove_if( [=]( const FileSystemEntry& entry ) { return entry.Media == media; } );
}

FileSystemObject* VirtualFileSystem::openFile( const fnString_t& filename, const int32_t mode )
{
    for ( auto& fileSystemEntry : fileSystemEntries ) {
        if ( filename.compare( 0, fileSystemEntry.MountPoint.length(), fileSystemEntry.MountPoint ) == 0 ) {
            auto absoluteFilename = fileSystemEntry.Media->resolveFilename( fileSystemEntry.MountPoint, filename );
            auto openedFile = fileSystemEntry.Media->openFile( absoluteFilename, mode );

            if ( openedFile != nullptr ) {
                return openedFile;
            }
        }
    }

    return nullptr;
}

bool VirtualFileSystem::fileExists( const fnString_t& filename )
{
    for ( auto& fileSystemEntry : fileSystemEntries ) {
        if ( filename.compare( 0, fileSystemEntry.MountPoint.length(), fileSystemEntry.MountPoint ) == 0 ) {
            auto absoluteFilename = fileSystemEntry.Media->resolveFilename( fileSystemEntry.MountPoint, filename );
            auto fileExists = fileSystemEntry.Media->fileExists( absoluteFilename );

            if ( fileExists ) {
                return true;
            }
        }
    }

    return false;
}
