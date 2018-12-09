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

class CommandList;
class RenderDevice;
class VertexArrayObject;
class Material;
class Texture;
class Buffer;
class Mesh;

#include <Core/Maths/AABB.h>
#include <Core/Maths/Transform.h>
#include <vector>

class Terrain
{
public:
                                        Terrain( const fnString_t& TerrainName = FLAN_STRING( "Terrain" ) );
                                        Terrain( Terrain& Terrain ) = default;
                                        Terrain& operator = ( Terrain& Terrain ) = default;
                                        ~Terrain();

    void                                create( RenderDevice* renderDevice, BaseAllocator* allocator, Material* terrainMaterial, const uint16_t* grassmapTexels, const uint16_t* splatmapTexels, const uint16_t* heightmapTexels, const uint32_t heightmapWidth, const uint32_t heightmapHeight );

    const VertexArrayObject*            getVertexArrayObject() const;
    Material*                           getMaterial();
    const uint32_t                      getIndiceCount() const;
    const AABB&                         getAxisAlignedBoundingBox() const;
    float*                              getHeightmapValues() const;
    float*                              getHeightmapValuesHeightScaled() const;
    
    float                               getHeightmapLowestVertex() const;
    float                               getHeightmapHighestVertex() const;

    // TODO Crap API for quick prototyping
    void                                setVertexHeight( const uint32_t vertexIndex, const float updatedHeight );
    void                                setVertexMaterial( const uint32_t vertexIndex, const int materialIndexBaseLayer, const int materialIndexOverlayLayer, const float overlayLayerStrength );
    void                                setGrassHeight( const uint32_t vertexIndex, const glm::vec3& grassColor, const float updatedGrassHeight );

    void uploadSplatmap( CommandList* cmdList );
    void uploadGrassmap( CommandList* cmdList );
    void uploadHeightmap( CommandList* cmdList );
    void uploadPatchBounds( CommandList* cmdList );
    void computePatchsBounds();

    Texture* getGrassMap() const { return grassmapTexture; }
    const bool needReupload() const { return isEditionInProgress; }

private:
    struct VertexLayout {
        glm::vec3 positionWorldSpace;
        glm::vec3 patchBoundsAndSkirtIndex;
        glm::vec2 texCoordinates;
    };

private:
    fnString_t                          name;
    Material*                           material;
    AABB                                aabb;

    uint32_t                            meshIndiceCount;
    uint32_t                            scalePatchX;
    uint32_t                            scalePatchY;
    uint32_t                            heightmapDimension;

    bool                                isEditionInProgress;
    float                               heightmapHighestVertex;
    float                               heightmapLowestVertex;

    float*                              editorHeightmap;

    float*                              heightmap;
    Texture*                            heightmapTexture;

    uint16_t*                           splatmap;
    Texture*                            splatmapTexture;

    float*                              grassmap;
    Texture*                            grassmapTexture;

    std::vector<uint32_t>               indices;
    std::vector<VertexLayout>           vertices;

    int                                 currentVboIndex;
    std::unique_ptr<Buffer>             vertexBuffer[2];
    std::unique_ptr<Buffer>             indiceBuffer;
    std::unique_ptr<VertexArrayObject>  vertexArrayObject[2];

private:
    void CalcYBounds( const glm::vec3& FLAN_RESTRICT bottomLeft, const glm::vec3& FLAN_RESTRICT topRight, glm::vec3& FLAN_RESTRICT output );
};
