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
#include "AtmosphereModule.h"

#include "AtmosphereSettings.h"
#include "AtmosphereConstants.h"

#include <Graphics/RenderModules/AutomaticExposureModule.h>

#include <Graphics/TextureSlotIndexes.h>
#include <Graphics/CBufferIndexes.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/CommandList.h>
#include <Core/Factory.h>

FLAN_DEV_VAR( dev_SunVerticalAngle, "Default Sun Vertical Angle (local)", 1.000f, float )
FLAN_DEV_VAR( dev_SunHorizontalAngle, "Default Sun Horizontal Angle (local)", 0.05f, float )
FLAN_DEV_VAR( dev_SunAngularRadius, "Default Sun Angular Radius (local)", static_cast<float>( kSunAngularRadius ), float )

AtmosphereModule::AtmosphereModule()
    : transmittanceTexture( nullptr )
    , scatteringTexture( nullptr )
    , irradianceTexture( nullptr )
    , sunVerticalAngle( dev_SunVerticalAngle )
    , sunHorizontalAngle( dev_SunHorizontalAngle )
    , sunAngularRadius( dev_SunAngularRadius )
{

}

AtmosphereModule::~AtmosphereModule()
{
    transmittanceTexture = nullptr;
    scatteringTexture = nullptr;
    irradianceTexture = nullptr;
}

fnPipelineMutableResHandle_t AtmosphereModule::renderAtmosphere( RenderPipeline* renderPipeline, bool renderSunDisk )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Atmosphere Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            auto mainRenderTarget = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) ); 
            
            if ( mainRenderTarget == -1 ) {
                RenderPassTextureDesc passRenderTargetDesc = {};
                passRenderTargetDesc.description.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
                passRenderTargetDesc.description.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
                passRenderTargetDesc.description.depth = 1;
                passRenderTargetDesc.description.samplerCount = 1;
                passRenderTargetDesc.description.mipCount = 1;
                passRenderTargetDesc.description.arraySize = 1;
                passRenderTargetDesc.useGlobalDimensions = true;
                passRenderTargetDesc.initialState = RenderPassTextureDesc::CLEAR;

                // Allocate Pass Render Target
                passData.output[0] = renderPipelineBuilder->allocateTexture( passRenderTargetDesc );

                renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ), passData.output[0] );
            } else {
                passData.output[0] = mainRenderTarget;
            }

            // Allocate Parameters Buffer
            BufferDesc passBuffer = {};
            passBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            passBuffer.Size = sizeof( parameters );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( passBuffer );

            BufferDesc cameraBuffer = {};
            cameraBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            cameraBuffer.Size = sizeof( Camera::Data );

            passData.buffers[1] = renderPipelineBuilder->allocateBuffer( cameraBuffer );

            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};
            passPipelineState.hashcode = renderSunDisk ? FLAN_STRING_HASH( "AtmospherePass" ) : FLAN_STRING_HASH( "AtmosphereNoSunDiskPass" );
            passPipelineState.vertexStage = FLAN_STRING( "SkyRendering" );
            passPipelineState.pixelStage = renderSunDisk ? FLAN_STRING( "SkyRendering" ) : FLAN_STRING( "SkyRenderingNoSunDisc" );
            passPipelineState.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            passPipelineState.rasterizerState.cullMode = flan::rendering::eCullMode::CULL_MODE_NONE;
            passPipelineState.depthStencilState.enableDepthTest = false;
            passPipelineState.depthStencilState.enableDepthWrite = false;
            passPipelineState.blendState.enableBlend = false;
            passPipelineState.rasterizerState.useTriangleCCW = true;

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // Sampler State
            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = flan::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[0] = renderPipelineBuilder->allocateSampler( bilinearSamplerDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Set CmdList Viewport
            auto passViewport = renderPipelineResources->getActiveViewport();
            cmdList->setViewportCmd( passViewport );

            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            auto atmosphereRenderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &atmosphereRenderTarget );

            // Unbind any VAO (since we'll generate vertices based on the vertex ID semantic)
            cmdList->unbindVertexArrayCmd();

            // Bind Parameters Buffer
            auto constantBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );

            auto autoExposureBuffer = renderPipelineResources->getWellKnownImportedResource<AutoExposureBuffer>()->exposureBuffer;
            autoExposureBuffer->bindReadOnly( cmdList, 16, SHADER_STAGE_PIXEL );

            // Update Parameters
            parameters.SunSizeX = tan( sunAngularRadius );
            parameters.SunSizeY = cos( sunAngularRadius * 4.0f );
            parameters.SunDirection = glm::vec3(
                            cos( dev_SunVerticalAngle ) * cos( dev_SunHorizontalAngle ),
                            cos( dev_SunVerticalAngle ) * sin( dev_SunHorizontalAngle ),
                            sin( dev_SunVerticalAngle )
                        );

            constantBuffer->updateAsynchronous( cmdList, &parameters, sizeof( parameters ) );
            constantBuffer->bind( cmdList, CBUFFER_INDEX_ATMOSPHERE );

            // Bind Camera Buffer
            auto cameraCbuffer = renderPipelineResources->getBuffer( passData.buffers[1] );
            auto passCamera = renderPipelineResources->getActiveCamera();
            cameraCbuffer->updateAsynchronous( cmdList, &passCamera, sizeof( Camera::Data ) );
            cameraCbuffer->bind( cmdList, 0, SHADER_STAGE_VERTEX );

            // Bind Textures
            scatteringTexture->bind( cmdList, TEXTURE_SLOT_INDEX_ATMOSPHERE_SCATTERING );
            irradianceTexture->bind( cmdList, TEXTURE_SLOT_INDEX_ATMOSPHERE_IRRADIANCE );
            transmittanceTexture->bind( cmdList, TEXTURE_SLOT_INDEX_ATMOSPHERE_TRANSMITTANCE );

            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, TEXTURE_SLOT_INDEX_ATMOSPHERE_SCATTERING );
            bilinearSampler->bind( cmdList, TEXTURE_SLOT_INDEX_ATMOSPHERE_IRRADIANCE );
            bilinearSampler->bind( cmdList, TEXTURE_SLOT_INDEX_ATMOSPHERE_TRANSMITTANCE );

            // Render fullscreen triangle
            cmdList->drawCmd( 3 );

            autoExposureBuffer->unbind( cmdList );
            cmdList->bindBackbufferCmd();
        }
    );

    renderPipeline->importWellKnownResource( &parameters );

    return RenderPass.output[0];
}

void AtmosphereModule::loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager )
{
    // Load precomputed table and shaders
    transmittanceTexture = graphicsAssetManager->getTexture( ATMOSPHERE_TRANSMITTANCE_TEXTURE_NAME );
    scatteringTexture = graphicsAssetManager->getTexture( ATMOSPHERE_SCATTERING_TEXTURE_NAME );
    irradianceTexture = graphicsAssetManager->getTexture( ATMOSPHERE_IRRADIANCE_TEXTURE_NAME );

    // Set Default Parameters
    parameters.EarthCenter = glm::vec3( 0.0, 0.0, -kBottomRadius / kLengthUnitInMeters );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( FLAN_STRING_HASH( "AtmosphereRenderPass" ), [=]( RenderPipeline* renderPipeline ) { return renderAtmosphere( renderPipeline ); } );
    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( FLAN_STRING_HASH( "AtmosphereNoSunDiskRenderPass" ), [=]( RenderPipeline* renderPipeline ) { return renderAtmosphere( renderPipeline, false ); } );

    //// TODO Not my job; should be editor only
    //if ( !Sys_FileIsValid( "AtmospherePS.cso" ) ) {
    //    PA_COUT << __FUNCTION__ << " >> Data recomputing is required (this might take some time)" << std::endl;
    //    
    //    // Precompute data and generate shaders if they don't exist yet
    //    AtmospherePrecomputePass precomputePass = {};
    //    precomputePass.Initialize( renderDevice, shaderStageManager );
    //    precomputePass.SetActive( renderDevice );
    //}
}
