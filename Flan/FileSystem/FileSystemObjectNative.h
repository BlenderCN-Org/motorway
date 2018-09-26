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

#include "FileSystemObject.h"

class FileSystemObjectNative final : public FileSystemObject
{
public:
                        FileSystemObjectNative( const fnString_t& objectPath );
                        ~FileSystemObjectNative();

    virtual void        open( const int32_t mode = flan::core::eFileOpenMode::FILE_OPEN_MODE_READ ) override;
    virtual void        close() override;
    virtual bool        isOpen() override;
    virtual bool        isGood() override;
    virtual uint64_t    tell() override;
    virtual uint64_t    getSize() override;
    virtual void        read( uint8_t* buffer, const uint64_t size ) override;
    virtual void        write( uint8_t* buffer, const uint64_t size ) override;
    virtual void        writeString( const std::string& string ) override;
    virtual void        writeString( const char* string, const std::size_t length ) override;
    virtual void        skip( const uint64_t byteCountToSkip ) override;
    virtual void        seek( const uint64_t byteCount, const flan::core::eFileReadDirection direction ) override;

private:
    int32_t             openedMode;
    std::fstream        nativeStream;
};
