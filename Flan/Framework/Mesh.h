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

class RenderDevice;
class VertexArrayObject;

#include "SubMesh.h"

#include <Core/Maths/AABB.h>
#include <Rendering/Buffer.h>

#include <vector>

class Mesh
{
public:
                                Mesh( const fnString_t& meshName = FLAN_STRING( "Mesh" ) );
                                Mesh( Mesh& mesh ) = default;
                                Mesh& operator = ( Mesh& mesh ) = default;
                                ~Mesh();

    void                        create( RenderDevice* renderDevice, const BufferDesc& vertexBufferDesc, const BufferDesc& indiceBufferDesc, const float* vertexBufferContent, const uint32_t* indiceBufferContent );
    void                        addSubMesh( SubMesh&& subMeshData );

    BoundingSphere              getBoundingSphere() const;
    const AABB&                 getAABB() const;
    fnStringHash_t              getHashcode() const;
    const VertexArrayObject*    getVertexArrayObject() const;

    void                        setName( const fnString_t& meshName );
    const fnString_t&           getName() const;
    const std::vector<SubMesh>& getSubMeshVector() const;
    std::vector<SubMesh>&       getSubMeshVectorRW();

    void                        reset();

private:
    fnString_t                          name;
    AABB                                aabb;
    std::unique_ptr<Buffer>             vertexBuffer;
    std::unique_ptr<Buffer>             indiceBuffer;
    std::unique_ptr<VertexArrayObject>  vertexArrayObject;
    std::vector<SubMesh>	            subMeshes;

    struct  {
        uint8_t canBeCulled : 1;
        uint8_t castShadows : 1;
    } flags;
};
