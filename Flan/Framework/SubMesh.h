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

#include <Core/Maths/BoundingSphere.h>
#include <Core/Maths/AABB.h>

class Material;
struct SubMesh
{
    Material*       material;
    uint32_t 	    indiceBufferOffset;
    uint32_t	    indiceCount;
    BoundingSphere  boundingSphere;

#if FLAN_DEVBUILD
    fnString_t      name;
    AABB            aabb;
#endif
};

#ifndef FLAN_DEVBUILD
static_assert( std::is_pod<SubMesh>(), "SubMesh is not POD" );
static_assert( sizeof( SubMesh ) % 16 == 0, "SunMesh is not memory aligned!" );
#endif
