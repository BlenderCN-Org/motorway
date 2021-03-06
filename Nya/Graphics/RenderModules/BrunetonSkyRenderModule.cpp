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

#include "Shared.h"
#include "BrunetonSkyRenderModule.h"

#include <Graphics/RenderPipeline.h>
#include <Graphics/ShaderCache.h>
#include <Graphics/GraphicsAssetCache.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include <Framework/Cameras/Camera.h>

#include "AtmosphereSettings.h"
#include "AtmosphereConstants.h"

using namespace nya::rendering;

BrunetonSkyRenderModule::BrunetonSkyRenderModule()
    : transmittanceTexture( nullptr )
    , scatteringTexture( nullptr )
    , irradianceTexture( nullptr )
    , sunVerticalAngle( 1.000f )
    , sunHorizontalAngle( 0.5f )
    , sunAngularRadius( static_cast<float>( kSunAngularRadius ) )
{

}

BrunetonSkyRenderModule::~BrunetonSkyRenderModule()
{
    transmittanceTexture = nullptr;
    scatteringTexture = nullptr;
    irradianceTexture = nullptr;
}

MutableResHandle_t BrunetonSkyRenderModule::renderSky( RenderPipeline* renderPipeline, const bool renderSunDisk, const bool useAutomaticExposure )
{
    struct PassData {
        MutableResHandle_t  output;

        ResHandle_t         parametersBuffer;
        ResHandle_t         cameraBuffer;
        ResHandle_t         autoExposureBuffer;
        ResHandle_t         bilinearSampler;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Sky Render Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            BufferDesc skyBufferDesc;
            skyBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            skyBufferDesc.size = sizeof( parameters );

            passData.parametersBuffer = renderPipelineBuilder.allocateBuffer( skyBufferDesc, SHADER_STAGE_PIXEL );

            BufferDesc cameraBufferDesc;
            cameraBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            cameraBufferDesc.size = sizeof( CameraData );

            passData.cameraBuffer = renderPipelineBuilder.allocateBuffer( cameraBufferDesc, SHADER_STAGE_VERTEX );

            TextureDescription rtDesc = {};
            rtDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            rtDesc.format = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

            passData.output = renderPipelineBuilder.allocateRenderTarget( rtDesc, RenderPipelineBuilder::USE_PIPELINE_DIMENSIONS | RenderPipelineBuilder::USE_PIPELINE_SAMPLER_COUNT );

            // Sampler State
            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );

            if ( useAutomaticExposure )
                passData.autoExposureBuffer = renderPipelineBuilder.retrievePersistentBuffer( NYA_STRING_HASH( "AutoExposure/ReadBuffer" ) );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice ) {
            // Update viewport (using image quality scaling)
            const CameraData* camera = renderPipelineResources.getMainCamera();

            nyaVec2f scaledViewportSize = camera->viewportSize * camera->imageQuality;
            Viewport vp;
            vp.X = 0;
            vp.Y = 0;
            vp.Width = static_cast<int>( scaledViewportSize.x );
            vp.Height = static_cast<int>( scaledViewportSize.y );
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;

            // Retrieve allocated resources
            Buffer* skyBuffer = renderPipelineResources.getBuffer( passData.parametersBuffer );

            // Update Parameters
            parameters.SunSizeX = tanf( sunAngularRadius );
            parameters.SunSizeY = cosf( sunAngularRadius * 4.0f );
            parameters.SunDirection = nyaVec3f(
                cosf( sunVerticalAngle ) * cosf( sunHorizontalAngle ),
                cosf( sunVerticalAngle ) * sinf( sunHorizontalAngle ),
                sinf( sunVerticalAngle )
            );

            Buffer* cameraBuffer = renderPipelineResources.getBuffer( passData.cameraBuffer );

            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );
            Buffer* autoExposureBuffer = renderPipelineResources.getPersistentBuffer( passData.autoExposureBuffer );

            // Render Pass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );

            RenderPass renderPass;
            renderPass.attachement[0] = { outputTarget, 0, 0 };

            // Pipeline State
            PipelineState* pipelineStateObject = getPipelineStatePermutation( camera->msaaSamplerCount, renderSunDisk, useAutomaticExposure );

            // Resource List
            ResourceList resourceList;
            resourceList.resource[0].buffer = skyBuffer;
            resourceList.resource[1].buffer = cameraBuffer;
            resourceList.resource[2].sampler = bilinearSampler;
            resourceList.resource[3].texture = scatteringTexture;
            resourceList.resource[4].texture = irradianceTexture;
            resourceList.resource[5].texture = transmittanceTexture;

            if ( renderSunDisk )
                resourceList.resource[6].buffer = autoExposureBuffer;

            renderDevice->updateResourceList( pipelineStateObject, resourceList );

            CommandList& cmdList = renderDevice->allocateGraphicsCommandList();
            {
                cmdList.begin();

                cmdList.setViewport( vp );

                cmdList.updateBuffer( skyBuffer, &parameters, sizeof( parameters ) );
                cmdList.updateBuffer( cameraBuffer, camera, sizeof( CameraData ) );

                cmdList.beginRenderPass( pipelineStateObject, renderPass );
                {
                    cmdList.bindPipelineState( pipelineStateObject );
                    cmdList.draw( 3 );
                }
                cmdList.endRenderPass();

                cmdList.end();
            }

            renderDevice->submitCommandList( &cmdList );
        }
    );

    return passData.output;
}

void BrunetonSkyRenderModule::destroy( RenderDevice* renderDevice )
{
    for ( uint32_t i = 0; i < 7u; i++ ) {
        renderDevice->destroyPipelineState( skyRenderPso[i] );
        renderDevice->destroyPipelineState( skyRenderNoSunFixedExposurePso[i] );
    }
}

void BrunetonSkyRenderModule::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.vertexShader = shaderCache->getOrUploadStage( "Atmosphere/BrunetonSky", SHADER_STAGE_VERTEX );
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "Atmosphere/BrunetonSky+NYA_FIXED_EXPOSURE", SHADER_STAGE_PIXEL );
    psoDesc.primitiveTopology = nya::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    psoDesc.rasterizerState.cullMode = nya::rendering::eCullMode::CULL_MODE_NONE;
    psoDesc.depthStencilState.enableDepthTest = false;
    psoDesc.depthStencilState.enableDepthWrite = false;
    psoDesc.blendState.enableBlend = false;
    psoDesc.rasterizerState.useTriangleCCW = false;

    // ResourceList
    psoDesc.resourceListLayout.resources[0] = { 1, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
    psoDesc.resourceListLayout.resources[1] = { 0, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };

    psoDesc.resourceListLayout.resources[2] = { 0, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER };

    psoDesc.resourceListLayout.resources[3] = { 0, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_TEXTURE };
    psoDesc.resourceListLayout.resources[4] = { 1, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_TEXTURE };
    psoDesc.resourceListLayout.resources[5] = { 2, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_TEXTURE };

    // RenderPass
    psoDesc.renderPassLayout.attachements[0].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPassLayout.attachements[0].bindMode = RenderPassLayoutDesc::WRITE;
    psoDesc.renderPassLayout.attachements[0].targetState = RenderPassLayoutDesc::CLEAR;
    psoDesc.renderPassLayout.attachements[0].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;
    psoDesc.renderPassLayout.attachements[0].clearValue[0] = 0.0f;
    psoDesc.renderPassLayout.attachements[0].clearValue[1] = 0.0f;
    psoDesc.renderPassLayout.attachements[0].clearValue[2] = 0.0f;
    psoDesc.renderPassLayout.attachements[0].clearValue[3] = 1.0f;
    psoDesc.renderPassLayout.attachements[0].sampleCount = 1u;

    skyRenderNoSunFixedExposurePso[0] = renderDevice->createPipelineState( psoDesc );

    // Allocate and cache pipeline state
    uint32_t samplerCount = 2u;
    for ( uint32_t i = 1u; i < 7u; i++ ) {
        psoDesc.renderPassLayout.attachements[0].sampleCount = samplerCount;
        skyRenderNoSunFixedExposurePso[i] = renderDevice->createPipelineState( psoDesc );

        samplerCount *= 2u;
    }

    psoDesc.pixelShader = shaderCache->getOrUploadStage( "Atmosphere/BrunetonSky+NYA_RENDER_SUN_DISC", SHADER_STAGE_PIXEL );
    psoDesc.resourceListLayout.resources[6] = { 3, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_GENERIC_BUFFER };

    skyRenderPso[0] = renderDevice->createPipelineState( psoDesc );

    // Allocate and cache pipeline state
    samplerCount = 2u;
    for ( uint32_t i = 1u; i < 7u; i++ ) {
        psoDesc.renderPassLayout.attachements[0].sampleCount = samplerCount;
        skyRenderPso[i] = renderDevice->createPipelineState( psoDesc );

        samplerCount *= 2u;
    }

    // Load precomputed table and shaders
    transmittanceTexture = graphicsAssetCache->getTexture( ATMOSPHERE_TRANSMITTANCE_TEXTURE_NAME );
    scatteringTexture = graphicsAssetCache->getTexture( ATMOSPHERE_SCATTERING_TEXTURE_NAME );
    irradianceTexture = graphicsAssetCache->getTexture( ATMOSPHERE_IRRADIANCE_TEXTURE_NAME );

    // Set Default Parameters
    parameters.EarthCenter = nyaVec3f( 0.0, 0.0, -kBottomRadius / kLengthUnitInMeters );
}

PipelineState* BrunetonSkyRenderModule::getPipelineStatePermutation( const uint32_t samplerCount, const bool renderSunDisk, const bool useAutomaticExposure )
{
    switch ( samplerCount ) {
    case 1:
        return ( renderSunDisk ) ? skyRenderPso[0] : skyRenderNoSunFixedExposurePso[0];
    case 2:
        return ( renderSunDisk ) ? skyRenderPso[1] : skyRenderNoSunFixedExposurePso[1];
    case 4:
        return ( renderSunDisk ) ? skyRenderPso[2] : skyRenderNoSunFixedExposurePso[2];
    case 8:
        return ( renderSunDisk ) ? skyRenderPso[3] : skyRenderNoSunFixedExposurePso[3];
    case 16:
        return ( renderSunDisk ) ? skyRenderPso[4] : skyRenderNoSunFixedExposurePso[4];
    case 32:
        return ( renderSunDisk ) ? skyRenderPso[5] : skyRenderNoSunFixedExposurePso[5];
    case 64:
        return ( renderSunDisk ) ? skyRenderPso[6] : skyRenderNoSunFixedExposurePso[6];
    default:
        return ( renderSunDisk ) ? skyRenderPso[0] : skyRenderNoSunFixedExposurePso[0];
    }
}
