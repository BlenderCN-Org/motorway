/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

#include "FileSystem.h"

class FileSystemObject;

class VirtualFileSystem
{
public:
                        VirtualFileSystem();
                        VirtualFileSystem( VirtualFileSystem& ) = delete;
                        VirtualFileSystem& operator = ( VirtualFileSystem& ) = delete;
                        ~VirtualFileSystem();

    void                mount( FileSystem* media, const fnString_t& mountPoint, const uint64_t mountOrder );
    void                unmount( FileSystem* media );

    FileSystemObject*   openFile( const fnString_t& filename, const int32_t mode = flan::core::eFileOpenMode::FILE_OPEN_MODE_READ );
    bool                fileExists( const fnString_t& filename );

private:
    struct FileSystemEntry {
        fnString_t  MountPoint;
        FileSystem* Media;
        uint64_t    MountOrder; // From 0 (most important fs; checked first when opening a file) to MAX_UINT64 (least important fs)
    };

private:
    std::list<FileSystemEntry>  fileSystemEntries;
};
