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
#pragma once

#include "FileOpenModes.h"
#include "FileReadDirections.h"

// Abstract object (file, directory, etc.)
class FileSystemObject
{
public:
    inline fnStringHash_t   getHashcode() const { return fileHashcode; }
    inline fnString_t       getFilename() const { return nativeObjectPath; }
    inline void             writePadding()
    {
        static constexpr uint8_t PADDING = 0xFF;

        auto streamPos = tell();
        while ( streamPos % 16 != 0 ) {
            write( ( uint8_t* )&PADDING, sizeof( char ) );
            streamPos++;
        }
    }

public:
    template<typename T>    void write( const T& variable ) { write( (uint8_t*)&variable, sizeof( T ) ); }
    template<typename T>    void read( const T& variable ) { read( (uint8_t*)&variable, sizeof( T ) ); }

public:
    virtual void            open( const int32_t mode = flan::core::eFileOpenMode::FILE_OPEN_MODE_READ ) = 0;
    virtual void            close() = 0;
    virtual bool            isOpen() = 0;
    virtual bool            isGood() = 0;
    virtual uint64_t        tell() = 0;
    virtual uint64_t        getSize() = 0;
    virtual void            read( uint8_t* buffer, const uint64_t size ) = 0;
    virtual void            write( uint8_t* buffer, const uint64_t size ) = 0;
    virtual void            writeString( const std::string& string ) = 0;
    virtual void            writeString( const char* string, const std::size_t length ) = 0;
    virtual void            skip( const uint64_t byteCountToSkip ) = 0;
    virtual void            seek( const uint64_t byteCount, const flan::core::eFileReadDirection direction ) = 0;

protected:
    fnStringHash_t          fileHashcode;
    fnString_t              nativeObjectPath;
};
