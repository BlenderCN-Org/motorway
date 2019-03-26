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
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
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
            cmdList->setViewport( vp );

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

            cmdList->updateBuffer( skyBuffer, &parameters, sizeof( parameters ) );

            Buffer* cameraBuffer = renderPipelineResources.getBuffer( passData.cameraBuffer );
            cmdList->updateBuffer( cameraBuffer, camera, sizeof( CameraData ) );

            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );
            Buffer* autoExposureBuffer = renderPipelineResources.getPersistentBuffer( passData.autoExposureBuffer );

            // Bind resources to the pipeline
            // Pipeline State
            PipelineState* pso = ( renderSunDisk ) ? skyRenderPso : skyRenderNoSunFixedExposurePso;
            cmdList->bindPipelineState( pso );

            // Resource List
            ResourceList resourceList;
            resourceList.resource[0].buffer = skyBuffer;
            resourceList.resource[1].buffer = cameraBuffer;
            resourceList.resource[2].sampler = bilinearSampler;
            resourceList.resource[3].sampler = bilinearSampler;
            resourceList.resource[4].sampler = bilinearSampler;

            if ( useAutomaticExposure )
                resourceList.resource[5].buffer = autoExposureBuffer;

            cmdList->bindResourceList( pso, resourceList );

            // Render Pass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );

            RenderPass renderPass;
            renderPass.resource[0].renderTarget = outputTarget;
            renderPass.resource[1].texture = scatteringTexture;
            renderPass.resource[2].texture = irradianceTexture;
            renderPass.resource[3].texture = transmittanceTexture;

            cmdList->bindRenderPass( pso, renderPass );
            cmdList->draw( 3 );
        }
    );

    return passData.output;
}

void BrunetonSkyRenderModule::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( skyRenderPso );
    renderDevice->destroyPipelineState( skyRenderNoSunFixedExposurePso );
}

void BrunetonSkyRenderModule::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.vertexShader = shaderCache->getOrUploadStage( "Atmosphere/BrunetonSky", SHADER_STAGE_VERTEX );
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "Atmosphere/BrunetonSky+NYA_RENDER_SUN_DISC", SHADER_STAGE_PIXEL );
    psoDesc.primitiveTopology = nya::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    psoDesc.rasterizerState.cullMode = nya::rendering::eCullMode::CULL_MODE_NONE;
    psoDesc.depthStencilState.enableDepthTest = false;
    psoDesc.depthStencilState.enableDepthWrite = false;
    psoDesc.blendState.enableBlend = false;
    psoDesc.rasterizerState.useTriangleCCW = false;

    // ResourceList
    psoDesc.resourceListBindings[0] = { 1, SHADER_STAGE_PIXEL, ResourceListBinding::RESOURCE_LIST_BINDING_TYPE_CBUFFER };
    psoDesc.resourceListBindings[1] = { 0, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, ResourceListBinding::RESOURCE_LIST_BINDING_TYPE_CBUFFER };

    psoDesc.resourceListBindings[2] = { 0, SHADER_STAGE_PIXEL, ResourceListBinding::RESOURCE_LIST_BINDING_TYPE_SAMPLER };
    psoDesc.resourceListBindings[3] = { 1, SHADER_STAGE_PIXEL, ResourceListBinding::RESOURCE_LIST_BINDING_TYPE_SAMPLER };
    psoDesc.resourceListBindings[4] = { 2, SHADER_STAGE_PIXEL, ResourceListBinding::RESOURCE_LIST_BINDING_TYPE_SAMPLER };

    psoDesc.resourceListBindings[5] = { 2, SHADER_STAGE_PIXEL, ResourceListBinding::RESOURCE_LIST_BINDING_TYPE_GENERIC_BUFFER };

    // RenderPass
    psoDesc.renderPass.attachements[0].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPass.attachements[0].bindMode = RenderPassDesc__::WRITE;
    psoDesc.renderPass.attachements[0].targetState = RenderPassDesc__::CLEAR_COLOR;
    psoDesc.renderPass.attachements[0].clearValue[0] = 0.0f;
    psoDesc.renderPass.attachements[0].clearValue[1] = 0.0f;
    psoDesc.renderPass.attachements[0].clearValue[2] = 0.0f;
    psoDesc.renderPass.attachements[0].clearValue[3] = 1.0f;

    psoDesc.renderPass.attachements[1].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPass.attachements[1].bindMode = RenderPassDesc__::READ;
    psoDesc.renderPass.attachements[1].targetState = RenderPassDesc__::IS_TEXTURE;

    psoDesc.renderPass.attachements[2].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPass.attachements[2].bindMode = RenderPassDesc__::READ;
    psoDesc.renderPass.attachements[2].targetState = RenderPassDesc__::IS_TEXTURE;

    psoDesc.renderPass.attachements[3].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPass.attachements[3].bindMode = RenderPassDesc__::READ;
    psoDesc.renderPass.attachements[3].targetState = RenderPassDesc__::IS_TEXTURE;

    // Allocate and cache pipeline state
    skyRenderPso = renderDevice->createPipelineState( psoDesc );

    psoDesc.pixelShader = shaderCache->getOrUploadStage( "Atmosphere/BrunetonSky+NYA_FIXED_EXPOSURE", SHADER_STAGE_PIXEL );
    skyRenderNoSunFixedExposurePso = renderDevice->createPipelineState( psoDesc );

    // Load precomputed table and shaders
    transmittanceTexture = graphicsAssetCache->getTexture( ATMOSPHERE_TRANSMITTANCE_TEXTURE_NAME );
    scatteringTexture = graphicsAssetCache->getTexture( ATMOSPHERE_SCATTERING_TEXTURE_NAME );
    irradianceTexture = graphicsAssetCache->getTexture( ATMOSPHERE_IRRADIANCE_TEXTURE_NAME );

    // Set Default Parameters
    parameters.EarthCenter = nyaVec3f( 0.0, 0.0, -kBottomRadius / kLengthUnitInMeters );
}
