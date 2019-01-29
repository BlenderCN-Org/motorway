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
class PoolAllocator;
class Material;
class VertexArrayObject;
class HosekSkyRenderModule;
class BrunetonSkyRenderModule;
class TextRenderingModule;
class ShaderCache;
class RenderPipeline;
class GraphicsAssetCache;

struct CameraData;
struct Viewport;
struct Buffer;

namespace glm
{
    enum qualifier;
    template<int i, int j, typename T, qualifier Q>
    struct mat;

    typedef mat<4, 4, float, (qualifier)0>	mat4;
}

struct DrawCommandKey
{
    enum Layer : uint8_t
    {
        LAYER_DEPTH,
        LAYER_WORLD,
        LAYER_HUD,
        LAYER_DEBUG
    };

    enum DepthViewportLayer : uint8_t
    {
        DEPTH_VIEWPORT_LAYER_DEFAULT,
        DEPTH_VIEWPORT_LAYER_CSM0,
        DEPTH_VIEWPORT_LAYER_CSM1,
        DEPTH_VIEWPORT_LAYER_CSM2,
        DEPTH_VIEWPORT_LAYER_CSM3
    };

    enum WorldViewportLayer : uint8_t
    {
        WORLD_VIEWPORT_LAYER_DEFAULT,
    };

    enum DebugViewportLayer : uint8_t
    {
        DEBUG_VIEWPORT_LAYER_DEFAULT,
    };

    enum HUDViewportLayer : uint8_t
    {
        HUD_VIEWPORT_LAYER_DEFAULT,
    };

    enum SortOrder : uint8_t
    {
        SORT_FRONT_TO_BACK = 0,
        SORT_BACK_TO_FRONT
    };

    union
    {
        struct
        {
            uint32_t materialSortKey; // material sort key (contains states and pipeline setup infos as a bitfield)

            uint16_t depth; // half float depth for distance sorting
            SortOrder sortOrder : 2; // front to back or back to front (opaque or transparent)
            uint8_t __PLACEHOLDER__ : 6;

            Layer layer : 2;
            uint8_t viewportLayer : 3;
            uint8_t viewportId : 3; // viewport dispatch index (should be managed by the builder)
        } bitfield;
        uint64_t value;
    };
};

struct DrawCommandInfos
{
    const Material*             material; // Geom cmd
    const Buffer*               vertexBuffer;
    const Buffer*               indiceBuffer;
    uint32_t                    indiceBufferOffset; // unused if indice buffer is null
    uint32_t                    indiceBufferCount; // same as above
    float                       alphaDitheringValue; // 0..1 (1.0f if disabled)
    uint32_t                    instanceCount; // 0 or 1 implicitly disable instancing
    const glm::mat4*            modelMatrix; // Points to a single matrix or an array of matrix (if instanciation is used)
};

struct DrawCmd
{
    DrawCommandKey      key;
    DrawCommandInfos    infos;

    static bool SortFrontToBack( const DrawCmd& cmd1, const DrawCmd& cmd2 )
    {
        return ( cmd1.key.value < cmd2.key.value );
    }

    static bool SortBackToFront( const DrawCmd& cmd1, const DrawCmd& cmd2 )
    {
        return ( cmd1.key.value > cmd2.key.value );
    }
};

class WorldRenderer
{
public:
                            WorldRenderer( BaseAllocator* allocator );
                            WorldRenderer( WorldRenderer& ) = delete;
                            WorldRenderer& operator = ( WorldRenderer& ) = delete;
                            ~WorldRenderer();

    void                    destroy( RenderDevice* renderDevice );

    void                    drawWorld( RenderDevice* renderDevice, const float deltaTime );
    DrawCmd&                allocateDrawCmd();

    void                    loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache );

    RenderPipeline&         allocateRenderPipeline( const Viewport& viewport, const CameraData* camera = nullptr );

    TextRenderingModule*     textRenderModule;
    BrunetonSkyRenderModule* skyRenderModule;

private:
    PoolAllocator*          drawCmdAllocator;

    uint32_t                renderPipelineCount;
    RenderPipeline*         renderPipelines;
};
