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

#include <Graphics/GraphicsAssetManager.h>
#include <Core/Factory.h>

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

void GrassRenderingModule::loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager )
{
    heightmapTestTexture = graphicsAssetManager->getTexture( FLAN_STRING( "GameData/Textures/heightmap_test.hmap" ) );
    grassMapTexture = graphicsAssetManager->getTexture( FLAN_STRING( "GameData/Textures/grassmap_test.dds" ) );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( FLAN_STRING_HASH( "TopDownTerrainCapture" ),
        [=]( RenderPipeline* renderPipeline ) {
            return addTopDownTerrainCapturePass( renderPipeline );
        } );
}

fnPipelineMutableResHandle_t GrassRenderingModule::addTopDownTerrainCapturePass( RenderPipeline* renderPipeline )
{
    struct PassBuffer
    {
        glm::vec3   positionWorldSpace;
        float       heightmapSize;
        glm::vec3   cameraPositionWorldSpace;
        uint32_t    __PADDING__;
    };
    FLAN_IS_MEMORY_ALIGNED( 16, PassBuffer );

    // TODO Retrieve the closest heightmap to the main viewport world position ONLY
    auto data = renderPipeline->addRenderPass(
        "Terrain Top Down Capture Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = FLAN_STRING_HASH( "TerrainTopDownPass" );
            passPipelineState.vertexStage = FLAN_STRING( "FullscreenTriangle" );
            passPipelineState.pixelStage = FLAN_STRING( "TopDownTerrainCapture" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            passPipelineState.rasterizerState.cullMode = flan::rendering::eCullMode::CULL_MODE_NONE;
            passPipelineState.depthStencilState.enableDepthTest = false;
            passPipelineState.depthStencilState.enableDepthWrite = false;

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            RenderPassTextureDesc passRenderTargetDesc = {};
            passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
            passRenderTargetDesc.description.depth = 1;
            passRenderTargetDesc.description.mipCount = 1;
            passRenderTargetDesc.description.samplerCount = 1;
            passRenderTargetDesc.description.arraySize = 1;
            passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            passRenderTargetDesc.description.width = 1024;
            passRenderTargetDesc.description.height = 1024;

            passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

            BufferDesc constantBuffer;
            constantBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            constantBuffer.Size = sizeof( PassBuffer );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( constantBuffer );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            auto viewport = cmdList->getViewportCmd();

            // Update viewport
            Viewport topDownDimensions = {
                0,
                0,
                1024,
                1024,
                0.0f,
                1.0f
            };
            cmdList->setViewportCmd( topDownDimensions );

            // Bind heightmap
            heightmapTestTexture->bind( cmdList, 0, SHADER_STAGE_PIXEL );

            const Camera::Data& cameraData = renderPipelineResources->getActiveCamera();

            PassBuffer bufferInfos;
            bufferInfos.heightmapSize = 512.0f;
            bufferInfos.cameraPositionWorldSpace = cameraData.worldPosition;
            bufferInfos.positionWorldSpace = glm::vec3( 0, 0, 0 );

            // Retrieve and update constant buffer
            auto constantBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            constantBuffer->updateAsynchronous( cmdList, &bufferInfos, sizeof( PassBuffer ) );
            constantBuffer->bind( cmdList, 0, SHADER_STAGE_PIXEL );

            // Set Ouput Target
            auto ouputRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &ouputRenderTarget );

            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            cmdList->unbindVertexArrayCmd();

            // Downsample
            cmdList->drawCmd( 3 );

            cmdList->bindBackbufferCmd();
            cmdList->setViewportCmd( viewport );

            constantBuffer->unbind( cmdList );
            ouputRenderTarget->unbind( cmdList );
    } );

    return data.output[0];
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
