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

MutableResHandle_t BrunetonSkyRenderModule::renderSky( RenderPipeline* renderPipeline, bool renderSunDisk )
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

            passData.autoExposureBuffer = renderPipelineBuilder.retrievePersistentBuffer( NYA_STRING_HASH( "AutoExposure/ReadBuffer" ) );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            // Resource List
            Buffer* skyBuffer = renderPipelineResources.getBuffer( passData.parametersBuffer );

            // Update Parameters
            parameters.SunSizeX = tan( sunAngularRadius );
            parameters.SunSizeY = cos( sunAngularRadius * 4.0f );
            parameters.SunDirection = nyaVec3f(
                cos( sunVerticalAngle ) * cos( sunHorizontalAngle ),
                cos( sunVerticalAngle ) * sin( sunHorizontalAngle ),
                sin( sunVerticalAngle )
            );

            cmdList->updateBuffer( skyBuffer, &parameters, sizeof( parameters ) );

            const CameraData* camera = renderPipelineResources.getMainCamera();

            Buffer* cameraBuffer = renderPipelineResources.getBuffer( passData.cameraBuffer );
            cmdList->updateBuffer( cameraBuffer, camera, sizeof( CameraData ) );

            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );
            Buffer* autoExposureBuffer = renderPipelineResources.getPersistentBuffer( passData.autoExposureBuffer );

            ResourceListDesc resListDesc = {};
            resListDesc.constantBuffers[0] = { 1, SHADER_STAGE_PIXEL, skyBuffer };
            resListDesc.constantBuffers[1] = { 0, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, cameraBuffer };

            resListDesc.buffers[0] = { 0, SHADER_STAGE_PIXEL, autoExposureBuffer };

            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };
            resListDesc.samplers[1] = { 1, SHADER_STAGE_PIXEL, bilinearSampler };
            resListDesc.samplers[2] = { 2, SHADER_STAGE_PIXEL, bilinearSampler };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // Render Pass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { outputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::CLEAR_COLOR, { 0 } };

            passDesc.attachements[1].texture = scatteringTexture;
            passDesc.attachements[1].stageBind = SHADER_STAGE_PIXEL;
            passDesc.attachements[1].bindMode = RenderPassDesc::READ;
            passDesc.attachements[1].targetState = RenderPassDesc::IS_TEXTURE;

            passDesc.attachements[2].texture = irradianceTexture;
            passDesc.attachements[2].stageBind = SHADER_STAGE_PIXEL;
            passDesc.attachements[2].bindMode = RenderPassDesc::READ;
            passDesc.attachements[2].targetState = RenderPassDesc::IS_TEXTURE;

            passDesc.attachements[3].texture = transmittanceTexture;
            passDesc.attachements[3].stageBind = SHADER_STAGE_PIXEL;
            passDesc.attachements[3].bindMode = RenderPassDesc::READ;
            passDesc.attachements[3].targetState = RenderPassDesc::IS_TEXTURE;

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            // Pipeline State
            cmdList->bindPipelineState( skyRenderPso );

            cmdList->draw( 3 );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.output;
}

void BrunetonSkyRenderModule::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( skyRenderPso );
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

    skyRenderPso = renderDevice->createPipelineState( psoDesc );

    // Load precomputed table and shaders
    transmittanceTexture = graphicsAssetCache->getTexture( ATMOSPHERE_TRANSMITTANCE_TEXTURE_NAME );
    scatteringTexture = graphicsAssetCache->getTexture( ATMOSPHERE_SCATTERING_TEXTURE_NAME );
    irradianceTexture = graphicsAssetCache->getTexture( ATMOSPHERE_IRRADIANCE_TEXTURE_NAME );

    // Set Default Parameters
    parameters.EarthCenter = nyaVec3f( 0.0, 0.0, -kBottomRadius / kLengthUnitInMeters );
}
