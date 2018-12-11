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
#include <Shared.h>
#include "RenderPipeline.h"

#include "GraphicsProfiler.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/CommandListPool.h>

#include <Core/TaskManager.h>
#include <Core/Profiler.h>

FLAN_DEV_VAR( EnablePipelineProfiling, "Enables GPU Profiling for RenderPipeline on Screen [false/true]", false, bool )

RenderPipeline::RenderPipeline( const bool shoudlRebuildEveryFrame )
    : renderPipelineBuilder( new RenderPipelineBuilder( this ) )
    , renderPipelineResources( new RenderPipelineResources() )
    , graphicsProfiler( new GraphicsProfiler() )
    , pipelineCmdListPool( nullptr )
    , renderPassCount( 0 )
    , rebuildEveryFrame( shoudlRebuildEveryFrame )
    , useProfiling( false )
{

}

RenderPipeline::~RenderPipeline()
{


}

void RenderPipeline::create( RenderDevice* renderDevice )
{
    pipelineCmdListPool.reset( new CommandListPool() );
    pipelineCmdListPool->create( renderDevice, 8 );
}

void RenderPipeline::enableProfiling( RenderDevice* renderDevice )
{
    graphicsProfiler->create( renderDevice );
    useProfiling = true;
}

void RenderPipeline::destroy( RenderDevice* renderDevice )
{
    renderPipelineResources->destroy( renderDevice );
    pipelineCmdListPool->destroy( renderDevice );
    graphicsProfiler->destroy( renderDevice );
}

RenderPassData& RenderPipeline::addRenderPass( const std::string& name, fnRenderPassSetup_t setup, fnRenderPassCallback_t execute )
{
    if ( renderPassCount >= MAX_RENDERPASS_COUNT ) {
        FLAN_CWARN << "Too many renderpasses for this frame (" << renderPassCount << " >= " << MAX_RENDERPASS_COUNT << ")" << std::endl;
        assert( false );
    }

    auto& renderPass = renderPasses.at( renderPassCount );
    renderPass.name = name;
    renderPass.setup = setup;
    renderPass.execute = execute;

    renderPassCount++;

    renderPass.setup( renderPipelineBuilder.get(), renderPass.data );

    return renderPass.data;
}

void RenderPipeline::addPipelineSetupPass( fnRenderPassPipelineSetup_t setup )
{
    setup( this, renderPipelineBuilder.get() );
}

void RenderPipeline::execute( RenderDevice* renderDevice, TaskManager* taskManager, ShaderStageManager* shaderStageManager )
{
    /*renderPipelineBuilder->allocateResourcesAndBuild( renderDevice, shaderStageManager, renderPipelineResources.get() );

    for ( uint32_t passIndex = 0; passIndex < renderPassCount; passIndex++ ) {
        auto& renderPass = renderPasses.at( passIndex );
        renderPass.execute( nullptr, renderPipelineResources.get(), renderPass.data );
    }

    renderPipelineResources->clearBuckets();

    // Reset the pass count if the pipeline is dynamically built
    if ( rebuildEveryFrame ) {
        renderPassCount = 0;
    }*/
}

void RenderPipeline::executeProfiled( RenderDevice* renderDevice, TaskManager* taskManager, ShaderStageManager* shaderStageManager )
{
    auto setupCmdList = pipelineCmdListPool->allocateCmdList( renderDevice );

    setupCmdList->beginCommandList( renderDevice );

    g_Profiler.beginSection( "RenderPipelineBuilder::allocateResourcesAndBuild" );
        renderPipelineBuilder->allocateResourcesAndBuild( renderDevice, setupCmdList, shaderStageManager, renderPipelineResources.get() );
    g_Profiler.endSection();

    setupCmdList->endCommandList( renderDevice );
    setupCmdList->playbackCommandList( renderDevice );

    if ( renderPassCount > 0 ) {
        std::vector<CommandList*> cmdLists;
        CommandList* cmdList = nullptr;
        cmdLists.push_back( pipelineCmdListPool->allocateCmdList( renderDevice ) );
        cmdList = cmdLists.back();
        cmdList->beginCommandList( renderDevice );
        for ( uint32_t passIndex = 0; passIndex < renderPassCount; passIndex++ ) {
           /* if ( ( passIndex % 8 ) == 0 ) {
                if ( cmdList != nullptr )
                    cmdList->endCommandList( renderDevice );

                cmdLists.push_back( pipelineCmdListPool->allocateCmdList( renderDevice ) );
                cmdList = cmdLists.back();
                cmdList->beginCommandList( renderDevice );
            }
*/
            auto& renderPass = renderPasses.at( passIndex );

            g_Profiler.beginSection( renderPass.name );
            graphicsProfiler->beginSection( cmdList, renderPass.name );
            renderPass.execute( cmdList, renderPipelineResources.get(), renderPass.data );
            graphicsProfiler->endSection( cmdList );
            g_Profiler.endSection();
        }
        cmdList->endCommandList( renderDevice );

        for ( auto cmdList : cmdLists ) {
            cmdList->playbackCommandList( renderDevice );
        }
    }

    renderPipelineResources->clearBuckets();

    // Reset the pass count if the pipeline is dynamically built
    if ( rebuildEveryFrame ) {
        renderPassCount = 0;
    }
}

void RenderPipeline::setTimeDelta( const float timeDelta )
{
    renderPipelineResources->setTimeDelta( timeDelta );
}

void RenderPipeline::printPassProfiling( RenderDevice* renderDevice, WorldRenderer* worldRenderer )
{
    g_Profiler.beginSection( "GraphicsProfiler::onFrame" );
    graphicsProfiler->drawOnScreen( EnablePipelineProfiling, 1.00f, 0.05f );
    graphicsProfiler->onFrame( renderDevice, worldRenderer );
    g_Profiler.endSection();
}

void RenderPipeline::setPipelineType( const bool shoudlRebuildEveryFrame )
{
    rebuildEveryFrame = shoudlRebuildEveryFrame;
}

void RenderPipeline::reset()
{
    rebuildEveryFrame = 0;
}

void RenderPipeline::setRendererViewport( const RenderPipelineViewport& viewport )
{
    currentViewport = viewport;

    renderPipelineBuilder->setViewport( currentViewport );
}

fnPipelineMutableResHandle_t RenderPipeline::importRenderTarget( RenderTarget* renderTarget )
{
    return renderPipelineResources->importRenderTarget( renderTarget );
}

RenderTarget* RenderPipeline::retrieveRenderTarget( fnPipelineResHandle_t resourceHandle )
{
    return renderPipelineResources->retrieveRenderTarget( resourceHandle );
}

void RenderPipeline::addToLayerBucket( const DrawCommandKey::Layer layer, const uint8_t viewportLayer, const DrawCommandInfos& geometryDrawCall )
{
    renderPipelineResources->addToLayerBucket( layer, viewportLayer, geometryDrawCall );
}
