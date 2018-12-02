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

#include "RenderPipelineBuilder.h"
#include "RenderPass.h"
#include "RenderPipelineResources.h"
#include "DrawCommand.h"

#include <array>

class GraphicsProfiler;
class RenderDevice;
class WorldRenderer;
class CommandListPool;
class TaskManager;

class RenderPipeline
{
public:
                                    RenderPipeline( const bool shoudlRebuildEveryFrame = false );
                                    RenderPipeline( RenderPipeline& ) = default;
                                    RenderPipeline& operator = ( RenderPipeline& ) = default;
                                    ~RenderPipeline();


    void                            create( RenderDevice* renderDevice );
    void                            enableProfiling( RenderDevice* renderDevice );
    void                            destroy( RenderDevice* renderDevice );
    RenderPassData&                 addRenderPass( const std::string& name, fnRenderPassSetup_t setup, fnRenderPassCallback_t execute );
    void                            addPipelineSetupPass( fnRenderPassPipelineSetup_t setup );
    void                            execute( RenderDevice* renderDevice, TaskManager* taskManager, ShaderStageManager* shaderStageManager );
    void                            executeProfiled( RenderDevice* renderDevice, TaskManager* taskManager, ShaderStageManager* shaderStageManager );
    void                            setPipelineType( const bool shoudlRebuildEveryFrame = false );
    void                            reset();
    void                            setRendererViewport( const RenderPipelineViewport& viewport );
    void                            setTimeDelta( const float timeDelta );

    void                            printPassProfiling( RenderDevice* renderDevice, WorldRenderer* worldRenderer );

    template<typename T>
    void importWellKnownResource( T* resource )
    {
        renderPipelineResources->importWellKnownResource<T>( resource );
    }

    void                            importWellKnownResource( const fnStringHash_t hashcode, void* resource );
    fnPipelineMutableResHandle_t    importRenderTarget( RenderTarget* renderTarget );
    RenderTarget*                   retrieveRenderTarget( fnPipelineResHandle_t resourceHandle );

    void                            addToLayerBucket( const DrawCommandKey::Layer layer, const uint8_t viewportLayer, const DrawCommandInfos& geometryDrawCall );

private:
    static constexpr int MAX_RENDERPASS_COUNT = 128;
    using RenderPasses_t = std::array<RenderPass, MAX_RENDERPASS_COUNT>;

private:
    std::unique_ptr<RenderPipelineBuilder> renderPipelineBuilder;
    std::unique_ptr<RenderPipelineResources> renderPipelineResources;

    std::unique_ptr<GraphicsProfiler>           graphicsProfiler;
    std::unique_ptr<CommandListPool>            pipelineCmdListPool;

    RenderPipelineViewport currentViewport;
    RenderPasses_t  renderPasses;
    uint32_t        renderPassCount;
    bool            rebuildEveryFrame;
    bool            useProfiling;
};
