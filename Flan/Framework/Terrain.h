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
class Material;
class Texture;
class Buffer;
class Mesh;

#include <Core/Maths/AABB.h>

struct GrassType
{
    // LOD0
    VertexArrayObject*  vaoGeometry;
    Material*           materialGeometry;

    // LOD1
    Material*           materialInspostor;
};

struct GrassGrid
{
    struct
    {
        GrassType*  type;
    } cells[512 * 512];
};

class Terrain
{
public:
                                        Terrain( const fnString_t& TerrainName = FLAN_STRING( "Terrain" ) );
                                        Terrain( Terrain& Terrain ) = default;
                                        Terrain& operator = ( Terrain& Terrain ) = default;
                                        ~Terrain();

    void                                create( RenderDevice* renderDevice, Material* terrainMaterial, Material* grassTest, const uint16_t* heightmapTexels, const uint32_t heightmapWidth, const uint32_t heightmapHeight );

    const VertexArrayObject*            getVertexArrayObject() const;
    Material*                           getMaterial();
    const uint32_t                      getIndiceCount() const;
    const AABB&                         getAxisAlignedBoundingBox() const;
    float*                              getHeightmapValues() const;
    float                               getHeightmapLowestVertex() const;
    float                               getHeightmapHighestVertex() const;

    Mesh* GRASS_TEST;

private:
    fnString_t                          name;
    Material*                           material;
    AABB                                aabb;

    uint32_t                            meshIndiceCount;

    float                               heightmapHighestVertex;
    float                               heightmapLowestVertex;

    float*                              heightmap;
    std::unique_ptr<Texture>            heightmapTexture;

    std::unique_ptr<Buffer>             vertexBuffer;
    std::unique_ptr<Buffer>             indiceBuffer;
    std::unique_ptr<VertexArrayObject>  vertexArrayObject;
};
