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
class OpaquePass;
class RenderableEntityManager;
class GraphicsAssetManager;
class VirtualFileSystem;
class Sampler;
class Texture;
class Buffer;
class PipelineState;
class ShaderStageManager;
class RenderTarget;
class RenderPipeline;
class TextRenderingModule;
class AtmosphereModule;
class AutomaticExposureModule;
class GrassRenderingModule;
class TaskManager;
class LineRenderingModule;
class BaseAllocator;
class PoolAllocator;
class LinearAllocator;

struct EnvironmentProbe;

#include <Framework/Cameras/Camera.h>
#include <Rendering/Viewport.h>

#include "DrawCommand.h"
#include "RenderPass.h"

#include <Core/Allocators/PoolAllocator.h>
#include <queue>

struct BRDFInputs
{
    Texture*        dfgLut;
    RenderTarget*   envProbeCapture;
    RenderTarget*   envProbeDiffuse;
    RenderTarget*   envProbeSpecular;
};

struct TerrainStreaming
{
    struct {
        glm::vec4 terrainSamplingParameters;
        uint32_t  terrainSampledSplatIndexes;
        uint32_t  __PADDING__[3];
    } terrainMaterialStreaming[256];

    Texture*        baseColorStreamed;
    Texture*        normalStreamed;
};

// Rendering gluing class
// This is a high level view of the rendering features (meaning that it should not contain API specific stuff)
class WorldRenderer
{
public:
                            WorldRenderer( BaseAllocator* allocator );
                            WorldRenderer( WorldRenderer& ) = delete;
                            WorldRenderer& operator = ( WorldRenderer& ) = delete;
                            ~WorldRenderer();

    void                    create( RenderDevice* activeRenderDevice );
    void                    destroy();
    void                    onFrame( const float interpolatedFrameTime, TaskManager* taskManager );

    void                    onResize( const unsigned int width, const unsigned int height );

    RenderPipelineViewport& addViewport( const int viewportIndex );
    void                    addDrawCommand( DrawCommand&& drawCmd );

    void                    drawDebugText( const std::string& text, const float scale, const float x, const float y, const float outlineThickness = 0.8f, const glm::vec4& color = glm::vec4( 1, 1, 1, 1 ), const bool useNormalizedCoordinates = true );
    void                    drawDebugLine( const glm::vec3& from, const glm::vec3& to, const float thickness, const glm::vec4& color );

    void                    loadCachedResources( BaseAllocator* baseAllocator, ShaderStageManager* shaderStageManager, GraphicsAssetManager* graphicsAssetManager );

    unsigned int            getFrameNumber() const;
    float                   getTimeDelta() const;

    // TODO Move this to an appropriate dedicated renderer?
    void                    saveTexture( RenderTarget* outputTarget, const fnString_t& outputTargetName );
    void                    precomputeVMF( Texture* normalMap, const float roughnessValue, RenderTarget* outputRoughnessMap );
    void                    precomputeVMF( Texture* normalMap, Texture* roughnessMap, RenderTarget* outputRoughnessMap, const bool thightPackedTexture = false );

    void                    setGrassMap( Texture* grassmap );

#if FLAN_DEVBUILD
    // TODO Nothing to do in the worldRenderer...
    Material* getWireframeMaterial() const;

    VertexArrayObject* getSpherePrimitive( uint32_t& indiceCount ) const;
    VertexArrayObject* getRectanglePrimitive( uint32_t& indiceCount ) const;
    VertexArrayObject* getCirclePrimitive( uint32_t& indiceCount ) const;
    VertexArrayObject* getBoxPrimitive( uint32_t& indiceCount ) const;
    VertexArrayObject* getConePrimitive( uint32_t& indiceCount ) const;
#endif

private:
    // Renderer infos (shared with GPU)
    struct {
        float           timeDelta;
        float           worldTime;
        unsigned int    frameNumber;
        int             __PADDING1__;

        unsigned int    backbufferWidth;
        unsigned int    backbufferHeight;
        int             __PADDING__[2];
    } renderInfos;

private:
    RenderDevice*                           renderDevice;
    ShaderStageManager*                     shaderStageManager;
    LinearAllocator*                        resourceAllocator;

    // NOTE If you wanna add more viewport, extend the drawcmd key so that the viewport id use more bits (atm > 3bits)
    RenderPipelineViewport                  viewportsToRender[8];
    int                                     viewportToRenderCount;

    // Draw Commands (geom, jobs, etc.)
    PoolAllocator*                          drawCommandsPool;

    std::vector<DrawCommand> drawCommands;               // (front to back cmds)
    std::vector<DrawCommand> transparentDrawCommands;    // (back to front cmds)

    // RenderModules; independant from any RenderPipeline
    RenderPipeline*          renderPipeline;
    TextRenderingModule*     textRenderingModule;
    AtmosphereModule*        atmosphereRenderingModule;
    AutomaticExposureModule* autoExposureModule;
    LineRenderingModule*     lineRenderingModule;
    GrassRenderingModule*    grassRenderingModule;

    // TODO Move this
    RenderTarget*           previousFrameRenderTarget;

    RenderTarget*           environmentProbes[3];
    Material*               wireframeMaterial;

    Texture*                terrainStreamedBaseColor;
    Texture*                terrainStreamedNormal;
    TerrainStreaming        terrainStreaming;

    Buffer*                 sphereVbo;
    Buffer*                 sphereIbo;
    VertexArrayObject*      sphereVao;
    uint32_t                sphereIndiceCount;

    Buffer*                 rectangleVbo;
    Buffer*                 rectangleIbo;
    VertexArrayObject*      rectangleVao;
    uint32_t                rectangleIndiceCount;

    Buffer*                 circleVbo;
    Buffer*                 circleIbo;
    VertexArrayObject*      circleVao;
    uint32_t                circleIndiceCount;

    Buffer*                 boxVbo;
    Buffer*                 boxIbo;
    VertexArrayObject*      boxVao;
    uint32_t                boxIndiceCount;

    Buffer*                 coneVbo;
    Buffer*                 coneIbo;
    VertexArrayObject*      coneVao;
    uint32_t                coneIndiceCount;

    BRDFInputs              brdfInputs;

private:
    void createRenderTargets( void );
    void createPrimitives( void );

    void sortDrawCmds( void );
    void updateRenderInfos( const float interpolatedFrametime );

    fnPipelineResHandle_t addProbeCaptureSavePass( RenderPipeline* renderPipeline );
    fnPipelineResHandle_t addProbeConvolutionPass( RenderPipeline* renderPipeline );
    fnPipelineResHandle_t addSSAAResolvePass( RenderPipeline* renderPipeline );

    fnPipelineResHandle_t addAAPass( RenderPipeline* renderPipeline, const int32_t samplerCount, const bool useTemporalAA );
};
