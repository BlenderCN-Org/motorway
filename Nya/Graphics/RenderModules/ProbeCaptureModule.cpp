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
#include "ProbeCaptureModule.h"

#include <Io/FontDescriptor.h>

#include <Graphics/RenderPipeline.h>
#include <Graphics/GraphicsAssetCache.h>
#include <Graphics/ShaderCache.h>

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

#include <Shaders/Shared.h>

#include <Graphics/RenderPasses/CopyRenderPass.h>

using namespace nya::rendering;

ProbeCaptureModule::ProbeCaptureModule()
    : capturedProbesArray( nullptr )
    , diffuseProbesArray( nullptr )
    , specularProbesArray( nullptr )
    , copyPso( nullptr )
    , convolutionPso( nullptr )
{

}

ProbeCaptureModule::~ProbeCaptureModule()
{

}

void ProbeCaptureModule::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyRenderTarget( capturedProbesArray );
    renderDevice->destroyRenderTarget( diffuseProbesArray );
    renderDevice->destroyRenderTarget( specularProbesArray );

    renderDevice->destroyPipelineState( copyPso );
    renderDevice->destroyPipelineState( convolutionPso );
}

void ProbeCaptureModule::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache )
{
    TextureDescription probeArrayDesc = {};
    probeArrayDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    probeArrayDesc.depth = 6;
    probeArrayDesc.width = IBL_PROBE_DIMENSION;
    probeArrayDesc.height = IBL_PROBE_DIMENSION;
    probeArrayDesc.arraySize = MAX_IBL_PROBE_COUNT;
    probeArrayDesc.format = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;
    probeArrayDesc.flags.isCubeMap = 1;

    capturedProbesArray = renderDevice->createRenderTarget2D( probeArrayDesc );

    probeArrayDesc.mipCount = 8;
    probeArrayDesc.flags.useHardwareMipGen = 1;

    diffuseProbesArray = renderDevice->createRenderTarget2D( probeArrayDesc );
    specularProbesArray = renderDevice->createRenderTarget2D( probeArrayDesc );

    PipelineStateDesc psoDesc = {};
    psoDesc.vertexShader = shaderCache->getOrUploadStage( "FullscreenTriangle", SHADER_STAGE_VERTEX );
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "CopyTexture+NYA_SAMPLE_FROM_RENDERPASS", SHADER_STAGE_PIXEL );
    psoDesc.primitiveTopology = nya::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    psoDesc.resourceListLayout.resources[0] =  { 0, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER };

    psoDesc.renderPassLayout.attachements[0].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPassLayout.attachements[0].bindMode = RenderPassLayoutDesc::WRITE;
    psoDesc.renderPassLayout.attachements[0].targetState = RenderPassLayoutDesc::DONT_CARE;
    psoDesc.renderPassLayout.attachements[0].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

    psoDesc.renderPassLayout.attachements[1].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPassLayout.attachements[1].bindMode = RenderPassLayoutDesc::READ;
    psoDesc.renderPassLayout.attachements[1].targetState = RenderPassLayoutDesc::DONT_CARE;
    psoDesc.renderPassLayout.attachements[1].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

    copyPso = renderDevice->createPipelineState( psoDesc );

    psoDesc = {};
    psoDesc.vertexShader = shaderCache->getOrUploadStage( "Editor/IBLProbeConvolution", SHADER_STAGE_VERTEX );
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "Editor/IBLProbeConvolution", SHADER_STAGE_PIXEL );
    psoDesc.primitiveTopology = nya::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

    psoDesc.resourceListLayout.resources[0] =  { 0, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER };
    psoDesc.resourceListLayout.resources[1] =  { 0, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };

    psoDesc.renderPassLayout.attachements[0].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPassLayout.attachements[0].bindMode = RenderPassLayoutDesc::WRITE;
    psoDesc.renderPassLayout.attachements[0].targetState = RenderPassLayoutDesc::DONT_CARE;
    psoDesc.renderPassLayout.attachements[0].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

    psoDesc.renderPassLayout.attachements[1].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPassLayout.attachements[1].bindMode = RenderPassLayoutDesc::WRITE;
    psoDesc.renderPassLayout.attachements[1].targetState = RenderPassLayoutDesc::DONT_CARE;
    psoDesc.renderPassLayout.attachements[1].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

    psoDesc.renderPassLayout.attachements[2].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPassLayout.attachements[2].bindMode = RenderPassLayoutDesc::READ;
    psoDesc.renderPassLayout.attachements[2].targetState = RenderPassLayoutDesc::DONT_CARE;
    psoDesc.renderPassLayout.attachements[2].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

    convolutionPso = renderDevice->createPipelineState( psoDesc );
}

void ProbeCaptureModule::importResourcesToPipeline( RenderPipeline* renderPipeline )
{
    renderPipeline->importPersistentRenderTarget( NYA_STRING_HASH( "IBL/CapturedProbesArray" ), capturedProbesArray );
    renderPipeline->importPersistentRenderTarget( NYA_STRING_HASH( "IBL/DiffuseProbesArray" ), diffuseProbesArray );
    renderPipeline->importPersistentRenderTarget( NYA_STRING_HASH( "IBL/SpecularProbesArray" ), specularProbesArray );
}

void ProbeCaptureModule::convoluteProbeFace( RenderPipeline* renderPipeline, const uint32_t probeArrayIndex, const uint16_t probeCaptureStep, const uint32_t mipLevel )
{
    uint32_t faceIndex = probeArrayIndex * 6u + probeCaptureStep;

    struct PassData {
        ResHandle_t input;

        ResHandle_t outputDiffuse;
        ResHandle_t outputSpecular;

        ResHandle_t passInfosBuffer;
        ResHandle_t bilinearSampler;
    };

    struct PassInfos {
        nyaVec3f    CubeDirectionX;
        float       RoughnessValue;

        nyaVec3f    CubeDirectionY;
        float       Width;

        nyaVec3f    CubeDirectionZ;
        uint32_t    ProbeIndex;
    };

    renderPipeline->addRenderPass<PassData>(
        "IBL Probe Convolution Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.outputDiffuse = renderPipelineBuilder.retrievePersistentRenderTarget( NYA_STRING_HASH( "IBL/DiffuseProbesArray" ) );
            passData.outputSpecular = renderPipelineBuilder.retrievePersistentRenderTarget( NYA_STRING_HASH( "IBL/SpecularProbesArray" ) );
            passData.input = renderPipelineBuilder.retrievePersistentRenderTarget( NYA_STRING_HASH( "IBL/CapturedProbesArray" ) );

            BufferDesc passInfosDesc = {};
            passInfosDesc.type = BufferDesc::CONSTANT_BUFFER;
            passInfosDesc.size = sizeof( PassInfos );

            passData.passInfosBuffer = renderPipelineBuilder.allocateBuffer( passInfosDesc, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL );

            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, CommandList* cmdList ) {
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            Buffer* infosBuffer = renderPipelineResources.getBuffer( passData.passInfosBuffer );

            PassInfos convolutionParameters;
            switch ( probeCaptureStep % 6 ) {
            case 0:
                convolutionParameters.CubeDirectionX = { 0, 0, -1 };
                convolutionParameters.CubeDirectionY = { 0, 1, 0 };
                convolutionParameters.CubeDirectionZ = { 1, 0, 0 };
                break;

            case 1:
                convolutionParameters.CubeDirectionX = { 0, 0, 1 };
                convolutionParameters.CubeDirectionY = { 0, 1, 0 };
                convolutionParameters.CubeDirectionZ = { -1, 0, 0 };
                break;

            case 2:
                convolutionParameters.CubeDirectionX = { 1, 0, 0 };
                convolutionParameters.CubeDirectionY = { 0, 0, -1 };
                convolutionParameters.CubeDirectionZ = { 0, 1, 0 };
                break;

            case 3:
                convolutionParameters.CubeDirectionX = { 1, 0, 0 };
                convolutionParameters.CubeDirectionY = { 0, 0, 1 };
                convolutionParameters.CubeDirectionZ = { 0, -1, 0 };
                break;

            case 4:
                convolutionParameters.CubeDirectionX = { 1, 0, 0 };
                convolutionParameters.CubeDirectionY = { 0, 1, 0 };
                convolutionParameters.CubeDirectionZ = { 0, 0, 1 };
                break;

            case 5:
                convolutionParameters.CubeDirectionX = { -1, 0, 0 };
                convolutionParameters.CubeDirectionY = { 0, 1, 0 };
                convolutionParameters.CubeDirectionZ = { 0, 0, -1 };
                break;
            };
            convolutionParameters.ProbeIndex = probeArrayIndex;
            convolutionParameters.Width = static_cast<float>( IBL_PROBE_DIMENSION >> mipLevel );
            convolutionParameters.RoughnessValue = mipLevel / nya::maths::max( 1.0f, std::log2( convolutionParameters.Width ) );

            cmdList->updateBuffer( infosBuffer, &convolutionParameters, sizeof( PassInfos ) );

            ResourceList resourceList;
            resourceList.resource[0].sampler = bilinearSampler;
            resourceList.resource[1].buffer = infosBuffer;

            // Viewport
            Viewport vp = {};
            vp.X = 0;
            vp.Y = 0;

            vp.Width = ( IBL_PROBE_DIMENSION >> mipLevel );
            vp.Height = ( IBL_PROBE_DIMENSION >> mipLevel );

            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;

            cmdList->setViewport( vp );

            // RenderPass
            RenderTarget* outputDiffuseTarget = renderPipelineResources.getPersitentRenderTarget( passData.outputDiffuse );
            RenderTarget* outputSpecularTarget = renderPipelineResources.getPersitentRenderTarget( passData.outputSpecular );
            RenderTarget* inputProbeArray = renderPipelineResources.getPersitentRenderTarget( passData.input );

            RenderPass renderPass;
            renderPass.attachement[0] = { outputDiffuseTarget, mipLevel, faceIndex };
            renderPass.attachement[1] = { outputSpecularTarget, mipLevel, faceIndex };
            renderPass.attachement[2] = { inputProbeArray, 0, 0 };

            cmdList->bindPipelineState( convolutionPso );
            cmdList->bindResourceList( convolutionPso, resourceList );
            cmdList->bindRenderPass( convolutionPso, renderPass );

            cmdList->draw( 4 );
        }
    );
}

void ProbeCaptureModule::saveCapturedProbeFace( RenderPipeline* renderPipeline, ResHandle_t capturedFace, const uint32_t probeArrayIndex, const uint16_t probeCaptureStep )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t output;
        ResHandle_t bilinearSampler;
    };

    const uint32_t faceIndex = probeArrayIndex * 6u + probeCaptureStep;

    renderPipeline->addRenderPass<PassData>(
        "IBL Captured Face Save Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            renderPipelineBuilder.setUncullablePass();

            passData.input = renderPipelineBuilder.readRenderTarget( capturedFace );

            passData.output = renderPipelineBuilder.retrievePersistentRenderTarget( NYA_STRING_HASH( "IBL/CapturedProbesArray" ) );

            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, CommandList* cmdList ) {
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            ResourceList resourceList;
            resourceList.resource[0].sampler = bilinearSampler;

            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getPersitentRenderTarget( passData.output );
            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );

            RenderPass renderPass;
            renderPass.attachement[0] = { outputTarget, 0, faceIndex };
            renderPass.attachement[1] = { inputTarget, 0, 0 };

            cmdList->bindPipelineState( copyPso );
            cmdList->bindResourceList( copyPso, resourceList );
            cmdList->bindRenderPass( copyPso, renderPass );

            cmdList->draw( 3 );
        }
    );
}
