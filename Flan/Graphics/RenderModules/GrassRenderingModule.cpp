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
#include "GrassRenderingModule.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/Texture.h>

#include <glm/gtc/matrix_transform.hpp>

GrassRenderingModule::GrassRenderingModule()
    : textureAllocator( nullptr )
    , grassMapTexture( nullptr )
    , randomnessTexture( nullptr )
{

}

GrassRenderingModule::~GrassRenderingModule()
{
    flan::core::free( textureAllocator, randomnessTexture );

    grassMapTexture = nullptr;
}

void GrassRenderingModule::create( RenderDevice* renderDevice, BaseAllocator* allocator )
{
    textureAllocator = allocator;

    float randomValues[64 * 64 * 4] = { 0.0f };
    for ( int i = 0; i < 64 * 64 * 4; i++ ) {
        randomValues[i] = static_cast< float >( rand() % 255 ) / 255.0f;
    }

    TextureDescription description;
    description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    description.arraySize = 1;
    description.depth = 1;
    description.format = IMAGE_FORMAT_R32G32B32A32_FLOAT;
    description.height = 64;
    description.width = 64;
    description.mipCount = 1;
    description.samplerCount = 1;

    randomnessTexture = flan::core::allocate<Texture>( allocator );
    randomnessTexture->createAsTexture2D( renderDevice, description, randomValues, sizeof( float ) * 64 * 64 * 4 );
    randomnessTexture->setResourceName( renderDevice, "Grass Random Noise" );
}

fnPipelineMutableResHandle_t GrassRenderingModule::addGrassGenerationPass( RenderPipeline* renderPipeline )
{
    auto data = renderPipeline->addRenderPass(
    "Grass Generation Pass",
    [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
        BufferDesc grassAppendBuffer;
        grassAppendBuffer.Type = BufferDesc::STRUCTURED_BUFFER;
        grassAppendBuffer.ViewFormat = IMAGE_FORMAT_R32_UINT;
        grassAppendBuffer.SingleElementSize = 1;
        grassAppendBuffer.Size = 48;
        grassAppendBuffer.Stride = 1;

        passData.buffers[0] = renderPipelineBuilder->allocateBuffer( grassAppendBuffer );
    },
    [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
        // Bind Pass Pipeline State
        auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
        cmdList->bindPipelineStateCmd( pipelineState );

        const auto& backbufferSize = renderPipelineResources->getActiveViewport();
        const auto& passCamera = renderPipelineResources->getActiveCamera();

        // Build Ortho Camera view in order to generate grass instances
        glm::mat4x4 orthoMatrix = glm::orthoLH( 0.0f, static_cast<float>( backbufferSize.Width ), static_cast<float>( backbufferSize.Height ), 0.0f, -1.0f, 1.0f );
        auto grassCameraPosition = passCamera.worldPosition + glm::vec3( 0.0f, 64.0f, 0.0f );
        glm::mat4x4 lookAtMatrix = glm::lookAt( grassCameraPosition, glm::vec3( 0, -1, 0 ), glm::vec3( 0, 1, 0 ) );


    } );

    return data.buffers[0];
}