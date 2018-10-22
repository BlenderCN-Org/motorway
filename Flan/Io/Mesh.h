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

class FileSystemObject;

#include <vector>

#include <Core/Maths/AABB.h>
#include <Core/Maths/BoundingSphere.h>

//=====================================
// Submesh entry
//=====================================

//=====================================
// Object holding geometry infos from a geometry
// file
//=====================================
struct GeomLoadData
{
    struct SubMesh
    {
        uint32_t        hashcode;
        uint32_t        indiceBufferOffset; // In basic machine units
        uint32_t        indiceCount;
        fnString_t      name;
        BoundingSphere  boundingSphere;
        AABB            aabb;
        uint32_t        levelOfDetailIndex;
    };

    std::vector<float>      vertices;
    std::vector<uint32_t>   indices;
    std::vector<uint32_t>   vertexStrides; // Scalars per component (e.g. a basic 3D position/2D uvmap would be { 3, 2 })

	std::vector<GeomLoadData::SubMesh>				subMesh;
	std::vector<std::pair<uint32_t, fnString_t>>	materialsReferences;

    GeomLoadData()
    {
        // TODO Use proper allocators instead of std (if needed)
        vertices.reserve( 8 * 1000 );
        indices.reserve( 8 * 1000 * 3 );
    }

    ~GeomLoadData()
    {
        vertices.clear();
        indices.clear();
        subMesh.clear();
        materialsReferences.clear();
    }

};

//=====================================
//  ReadGeometryFile
//      Read a geometry file (.mesh).
//      NOTE This function only return the file's data; you still need to build the mesh
//
//      Parameters:
//          fileName: the geometry file to load
//          data: a reference to a GeomLoadData object.
//=====================================
namespace flan
{
    namespace core
    {
        void    LoadGeometryFile( FileSystemObject* file, GeomLoadData& data );
    }
}
