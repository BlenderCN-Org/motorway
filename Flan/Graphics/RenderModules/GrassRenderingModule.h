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

#include <Rendering/PipelineState.h>
#include <Graphics/RenderPipeline.h>

class Texture;
class RenderDevice;
class RenderPipeline;
class GraphicsAssetManager;

class GrassRenderingModule
{
public:
                    GrassRenderingModule();
                    GrassRenderingModule( GrassRenderingModule& ) = default;
                    GrassRenderingModule& operator = ( GrassRenderingModule& ) = default;
                    ~GrassRenderingModule();

    void            create( RenderDevice* renderDevice, BaseAllocator* allocator );
    void            loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager );

    // TODO this is bad
    void            setGrassMap( Texture* grassmap ) { grassMapTexture = grassmap; }

private:
    BaseAllocator*  textureAllocator;
    Texture*        grassMapTexture;

    Texture*        grassAlbedoTest;

    Texture*        randomnessTexture;
    
    RenderTarget*   topDownRenderTarget;
    bool            isTopDownCaptured;

private:
    fnPipelineResHandle_t           addGrassSetupPass( RenderPipeline* renderPipeline );

    fnPipelineMutableResHandle_t    addTopDownTerrainCapturePass( RenderPipeline* renderPipeline );
    fnPipelineMutableResHandle_t    addGrassGenerationPass( RenderPipeline* renderPipeline );
    fnPipelineMutableResHandle_t    addIndirectDrawSetupPass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t instanceBuffer );
    fnPipelineMutableResHandle_t    addGrassRenderPass( RenderPipeline* renderPipeline, const bool enableMSAA );
};
