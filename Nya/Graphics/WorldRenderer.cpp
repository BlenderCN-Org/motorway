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
#include "RenderPasses/PresentRenderPass.h"

WorldRenderer::WorldRenderer( BaseAllocator* allocator )
    : renderPipelineCount( 0u )
    , drawCmdAllocator( nya::core::allocate<PoolAllocator>( allocator, sizeof( DrawCmd ), 4, sizeof( DrawCmd ) * 8192, allocator->allocate( sizeof( DrawCmd ) * 8192 ) ) )
    , textRenderModule( nya::core::allocate<TextRenderingModule>( allocator ) )
    , skyRenderModule( nya::core::allocate<BrunetonSkyRenderModule>( allocator ) )
    , renderPipelines( nya::core::allocateArray<RenderPipeline>( allocator, 8 ) )
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

    // Free render modules resources
    textRenderModule->destroy( renderDevice );
    skyRenderModule->destroy( renderDevice );

    FreeCachedResourcesPP( renderDevice );
}

void WorldRenderer::drawWorld( RenderDevice* renderDevice, const float deltaTime )
{
    // Execute pipelines linearly
    // TODO Could it be parallelized?
    for ( uint32_t pipelineIdx = 0; pipelineIdx < renderPipelineCount; pipelineIdx++ ) {
        renderPipelines[pipelineIdx].execute( renderDevice );
    }

    // Reset DrawCmd Pool
    drawCmdAllocator->clear();
    renderPipelineCount = 0;
}

DrawCmd& WorldRenderer::allocateDrawCmd()
{
    return *nya::core::allocate<DrawCmd>( drawCmdAllocator );
}

void WorldRenderer::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache )
{
    // Load render modules resources (cached pipeline states, LUTs, precomputed data tables, etc.)
    skyRenderModule->loadCachedResources( renderDevice, shaderCache, graphicsAssetCache );
    textRenderModule->loadCachedResources( renderDevice, shaderCache, graphicsAssetCache );

    LoadCachedResourcesPP( renderDevice, shaderCache );
}

RenderPipeline& WorldRenderer::allocateRenderPipeline( const Viewport& viewport, const CameraData* camera )
{
    RenderPipeline& renderPipeline = renderPipelines[renderPipelineCount++];
    renderPipeline.setViewport( viewport, camera );
    return renderPipeline;
}
