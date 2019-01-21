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

class FileSystemObject;

class FileSystem
{
public:
    virtual FileSystemObject*   openFile( const nyaString_t& filename, const int32_t mode = nya::core::eFileOpenMode::FILE_OPEN_MODE_READ ) = 0;
    virtual void                closeFile( FileSystemObject* fileSystemObject ) = 0;
    virtual void                createFolder( const nyaString_t& folderName ) = 0;
    virtual bool                fileExists( const nyaString_t& filename ) = 0;
    virtual bool                isReadOnly() = 0;
    virtual nyaString_t          resolveFilename( const nyaString_t& mountPoint, const nyaString_t& filename ) = 0;
};
