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

#include "Shared.h"
#include "Collider.h"

#include <FileSystem/FileSystemObject.h>

void Io_ReadConvexHullColliderFile( FileSystemObject* file, std::vector<float>& data )
{
    uint32_t vertexCount = 0;

    file->read( (uint8_t*)&vertexCount, sizeof( uint32_t ) );

    if ( vertexCount == 0 ) {
        FLAN_CERR << file->getFilename() << " : vertex count = 0" << std::endl;
    } else {
        data.resize( vertexCount );

        // Read buffer data
        file->read( (uint8_t*)&data.front(), vertexCount );
    }

    file->close();
}
