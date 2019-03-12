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
#include "WorldRenderer.h"

#include <Core/Allocators/PoolAllocator.h>

#include "RenderModules/BrunetonSkyRenderModule.h"
#include "RenderModules/TextRenderingModule.h"
#include "RenderModules/AutomaticExposureRenderModule.h"
#include "RenderPasses/PresentRenderPass.h"
#include "RenderPasses/FinalPostFxRenderPass.h"
#include "RenderPasses/BlurPyramidRenderPass.h"
#include "RenderPasses/CopyRenderPass.h"
#include "RenderPasses/MSAAResolveRenderPass.h"
#include "RenderModules/ProbeCaptureModule.h"

#include "PrimitiveCache.h"

static constexpr size_t MAX_DRAW_CMD_COUNT = 8192;

void RadixSort( DrawCmd* _keys, DrawCmd* _tempKeys, const size_t size )
{
    NYA_PROFILE_FUNCTION

    static constexpr size_t RADIXSORT_BITS = 11;
    static constexpr size_t RADIXSORT_HISTOGRAM_SIZE = ( 1 << RADIXSORT_BITS );
    static constexpr size_t RADIXSORT_BIT_MASK = ( RADIXSORT_HISTOGRAM_SIZE - 1 );

    DrawCmd* keys = _keys;
    DrawCmd* tempKeys = _tempKeys;

    uint32_t histogram[RADIXSORT_HISTOGRAM_SIZE];
    uint16_t shift = 0;
    uint32_t pass = 0;
    for ( ; pass < 6; ++pass ) {
        memset( histogram, 0, sizeof( uint32_t ) * RADIXSORT_HISTOGRAM_SIZE );

        bool sorted = true;
        {
            uint64_t key = keys[0].key.value;
            uint64_t prevKey = key;
            for ( uint32_t ii = 0; ii < size; ++ii, prevKey = key ) {
                key = keys[ii].key.value;

                uint16_t index = ( ( key >> shift ) & RADIXSORT_BIT_MASK );
                ++histogram[index];

                sorted &= ( prevKey <= key );
            }
        }

        if ( sorted ) {
            goto done;
        }

        uint32_t offset = 0;
        for ( uint32_t ii = 0; ii < RADIXSORT_HISTOGRAM_SIZE; ++ii ) {
            uint32_t count = histogram[ii];
            histogram[ii] = offset;

            offset += count;
        }

        for ( uint32_t ii = 0; ii < size; ++ii ) {
            uint64_t key = keys[ii].key.value;
            uint16_t index = ( ( key >> shift ) & RADIXSORT_BIT_MASK );
            uint32_t dest = histogram[index]++;

            tempKeys[dest] = keys[ii];
        }

        DrawCmd* swapKeys = tempKeys;
        tempKeys = keys;
        keys = swapKeys;

        shift += RADIXSORT_BITS;
    }

done:
    if ( ( pass & 1 ) != 0 ) {
        memcpy( _keys, _tempKeys, size * sizeof( DrawCmd ) );
    }
}

WorldRenderer::WorldRenderer( BaseAllocator* allocator )
    : primitiveCache( nya::core::allocate<PrimitiveCache>( allocator, allocator ) )
    , renderPipelineCount( 0u )
    , drawCmdAllocator( nya::core::allocate<PoolAllocator>( allocator, sizeof( DrawCmd ), 4, sizeof( DrawCmd ) * MAX_DRAW_CMD_COUNT, allocator->allocate( sizeof( DrawCmd ) * 8192 ) ) )
    , TextRenderModule( nya::core::allocate<TextRenderingModule>( allocator ) )
    , SkyRenderModule( nya::core::allocate<BrunetonSkyRenderModule>( allocator ) )
    , automaticExposureModule( nya::core::allocate<AutomaticExposureModule>( allocator ) )
    , probeCaptureModule( nya::core::allocate<ProbeCaptureModule>( allocator ) )
    , renderPipelines( nya::core::allocateArray<RenderPipeline>( allocator, 8, allocator ) )
{

}

WorldRenderer::~WorldRenderer()
{

}

void WorldRenderer::destroy( RenderDevice* renderDevice )
{
    for ( uint32_t i = 0; i < 8; i++ ) {
        renderPipelines[i].destroy( renderDevice );
    }

    primitiveCache->destroy( renderDevice );

    // Free render modules resources
    TextRenderModule->destroy( renderDevice );
    SkyRenderModule->destroy( renderDevice );
    automaticExposureModule->destroy( renderDevice );
    probeCaptureModule->destroy( renderDevice );

    FreeCachedResourcesCP( renderDevice );
    FreeCachedResourcesBP( renderDevice );
    FreeCachedResourcesFP( renderDevice );
    FreeCachedResourcesPP( renderDevice );
    FreeCachedResourcesMRP( renderDevice );
}

void WorldRenderer::drawWorld( RenderDevice* renderDevice, const float deltaTime )
{
    NYA_PROFILE_FUNCTION

    DrawCmd* drawCmds = static_cast<DrawCmd*>( drawCmdAllocator->getBaseAddress() );
    const size_t drawCmdCount = drawCmdAllocator->getAllocationCount();

    DrawCmd tmpDrawCmds[MAX_DRAW_CMD_COUNT];
    RadixSort( drawCmds, tmpDrawCmds, drawCmdCount );

    // Execute pipelines linearly
    // TODO Could it be parallelized?
    for ( uint32_t pipelineIdx = 0; pipelineIdx < renderPipelineCount; pipelineIdx++ ) {
        renderPipelines[pipelineIdx].submitAndDispatchDrawCmds( drawCmds, drawCmdCount );
        renderPipelines[pipelineIdx].execute( renderDevice, deltaTime );

#if NYA_DEVBUILD
        const char* profilingString = renderPipelines[pipelineIdx].getProfilingSummary();

        if ( profilingString != nullptr ) {
            TextRenderModule->addOutlinedText( profilingString, 0.35f, 10.0f + 200.0f * pipelineIdx, 96.0f );
        }
#endif
    }

    // Reset DrawCmd Pool
    drawCmdAllocator->clear();
    renderPipelineCount = 0;
}

DrawCmd& WorldRenderer::allocateDrawCmd()
{
    return *nya::core::allocate<DrawCmd>( drawCmdAllocator );
}

DrawCmd& WorldRenderer::allocateSpherePrimitiveDrawCmd()
{
    DrawCmd& cmd = allocateDrawCmd();

    const auto& spherePrimitive = primitiveCache->getSpherePrimitive();

    DrawCommandInfos& infos = cmd.infos;
    infos.vertexBuffer = spherePrimitive.vertexBuffer;
    infos.indiceBuffer = spherePrimitive.indiceBuffer;
    infos.indiceBufferOffset = spherePrimitive.indiceBufferOffset;
    infos.indiceBufferCount = spherePrimitive.indiceCount;
    infos.alphaDitheringValue = 1.0f;

    return cmd;
}

void WorldRenderer::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache )
{
    // Load render modules resources (cached pipeline states, LUTs, precomputed data tables, etc.)
    SkyRenderModule->loadCachedResources( renderDevice, shaderCache, graphicsAssetCache );
    TextRenderModule->loadCachedResources( renderDevice, shaderCache, graphicsAssetCache );
    automaticExposureModule->loadCachedResources( renderDevice, shaderCache, graphicsAssetCache );
    probeCaptureModule->loadCachedResources( renderDevice, shaderCache, graphicsAssetCache );

    LoadCachedResourcesFP( renderDevice, shaderCache );
    LoadCachedResourcesPP( renderDevice, shaderCache );
    LoadCachedResourcesBP( renderDevice, shaderCache );
    LoadCachedResourcesCP( renderDevice, shaderCache );
    LoadCachedResourcesMRP( renderDevice, shaderCache );

    primitiveCache->createPrimitivesBuffer( renderDevice );

#if NYA_DEVBUILD
    for ( int i = 0; i < 8; i++ ) {
        renderPipelines[i].enableProfiling( renderDevice );
    }
#endif
}

RenderPipeline& WorldRenderer::allocateRenderPipeline( const Viewport& viewport, const CameraData* camera )
{
    RenderPipeline& renderPipeline = renderPipelines[renderPipelineCount++];
    renderPipeline.setViewport( viewport, camera );
    return renderPipeline;
}
