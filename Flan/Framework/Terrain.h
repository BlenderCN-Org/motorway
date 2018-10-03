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

class RenderDevice;
class VertexArrayObject;
class Material;

#include <Core/Maths/AABB.h>
#include <Rendering/Buffer.h>

#include <vector>

class Terrain
{
public:
                                Terrain( const fnString_t& TerrainName = FLAN_STRING( "Terrain" ) );
                                Terrain( Terrain& Terrain ) = default;
                                Terrain& operator = ( Terrain& Terrain ) = default;
                                ~Terrain();

    void                        create( RenderDevice* renderDevice, Material* terrainMaterial );

    const VertexArrayObject*    getVertexArrayObject() const;
    Material*                   getMaterial();
    const uint32_t              getIndiceCount() const;

private:
    fnString_t                          name;
    Material*                           material;

    std::unique_ptr<Buffer>             vertexBuffer;
    std::unique_ptr<Buffer>             indiceBuffer;
    std::unique_ptr<VertexArrayObject>  vertexArrayObject;
};