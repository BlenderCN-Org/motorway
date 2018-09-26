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

#if FLAN_WIN
#include "FileSystemWin32.h"


bool flan::core::FileExistsImpl( const fnString_t& filename )
{
    const DWORD fileAttributes = GetFileAttributes( filename.c_str() );

    return !( fileAttributes == INVALID_FILE_ATTRIBUTES )
        && !( ( fileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY );
}

void flan::core::CreateFolderImpl( const fnString_t& folderName )
{
    CreateDirectory( folderName.c_str(), nullptr );
}
#endif
