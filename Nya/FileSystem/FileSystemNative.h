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

#include "FileSystem.h"

#include <list>

class FileSystemObjectNative;

class FileSystemNative final : public FileSystem
{
public:
                                        FileSystemNative( const nyaString_t& customWorkingDirectory = NYA_STRING( "" ) );
                                        FileSystemNative( FileSystemNative& ) = delete;
                                        FileSystemNative& operator = ( FileSystemNative& ) = delete;
                                        ~FileSystemNative() override;

    virtual FileSystemObject*           openFile( const nyaString_t& filename, const int32_t mode = nya::core::eFileOpenMode::FILE_OPEN_MODE_READ ) override;
    virtual void                        closeFile( FileSystemObject* fileSystemObject ) override;
    virtual void                        createFolder( const nyaString_t& folderName ) override;
    virtual bool                        fileExists( const nyaString_t& filename ) override;
    virtual bool                        isReadOnly() override;
    virtual nyaString_t                 resolveFilename( const nyaString_t& mountPoint, const nyaString_t& filename );

private:
    nyaString_t                          workingDirectory;
    std::list<FileSystemObjectNative*>  openedFiles;
};
