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
#include "FileSystemObjectNative.h"

#include <fstream>

using namespace nya::core;

FileSystemObjectNative::FileSystemObjectNative( const nyaString_t& objectPath )
    : openedMode( eFileOpenMode::FILE_OPEN_MODE_NONE )
{
    nativeObjectPath = objectPath;
    fileHashcode = CRC32( nativeObjectPath );
}

FileSystemObjectNative::~FileSystemObjectNative()
{
    close();

    nativeObjectPath = NYA_STRING( "" );
}

void FileSystemObjectNative::open( const int32_t mode )
{
    nativeStream.open( nativeObjectPath, static_cast<std::ios_base::openmode>( mode ) );

    openedMode = mode;
}

void FileSystemObjectNative::close()
{
    nativeStream.close();

    openedMode = eFileOpenMode::FILE_OPEN_MODE_NONE;
}

bool FileSystemObjectNative::isOpen()
{
    return nativeStream.is_open();
}

bool FileSystemObjectNative::isGood()
{
    return nativeStream.good();
}

uint64_t FileSystemObjectNative::tell()
{
    return ( ( openedMode & eFileOpenMode::FILE_OPEN_MODE_READ ) == eFileOpenMode::FILE_OPEN_MODE_READ ) ? nativeStream.tellg() : nativeStream.tellp();
}

uint64_t FileSystemObjectNative::getSize()
{
    auto fileOffset = nativeStream.tellg();

    nativeStream.seekg( 0, std::ios_base::end );
    uint64_t contentLength = nativeStream.tellg();
    nativeStream.seekg( fileOffset, std::ios_base::beg );

    return contentLength;
}

void FileSystemObjectNative::read( uint8_t* buffer, const uint64_t size )
{
    nativeStream.read( (char*)buffer, size );
}

void FileSystemObjectNative::write( uint8_t* buffer, const uint64_t size )
{
    nativeStream.write( (char*)buffer, size );
}

void FileSystemObjectNative::writeString( const std::string& string )
{
    writeString( string.c_str(), string.length() );
}

void FileSystemObjectNative::writeString( const char* string, const std::size_t length )
{
    nativeStream.write( string, length );
}

void FileSystemObjectNative::skip( const uint64_t byteCountToSkip )
{
    nativeStream.ignore( byteCountToSkip );
}

void FileSystemObjectNative::seek( const uint64_t byteCount, const eFileReadDirection direction )
{
    static constexpr decltype( std::ios::beg ) FRD_TO_SEEKDIR[3] = {
        std::ios::beg,
        std::ios::cur,
        std::ios::end,
    };

    if ( ( openedMode & eFileOpenMode::FILE_OPEN_MODE_READ ) == eFileOpenMode::FILE_OPEN_MODE_READ ) {
        nativeStream.seekg( byteCount, FRD_TO_SEEKDIR[direction] );
    } else {
        nativeStream.seekp( byteCount, FRD_TO_SEEKDIR[direction] );
    }
}
