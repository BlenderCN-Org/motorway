#include <Shared.h>
#include "CopyRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/CommandList.h>

static PipelineState*  g_PipelineStateObject = nullptr;

void LoadCachedResourcesCP( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.vertexShader = shaderCache->getOrUploadStage( "FullscreenTriangle", SHADER_STAGE_VERTEX );
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "CopyTexture", SHADER_STAGE_PIXEL );
    psoDesc.primitiveTopology = nya::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    g_PipelineStateObject = renderDevice->createPipelineState( psoDesc );
}

void FreeCachedResourcesCP( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( g_PipelineStateObject );
}

ResHandle_t AddCopyRenderPass( RenderPipeline* renderPipeline, ResHandle_t inputRenderTarget )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t output;
        ResHandle_t bilinearSampler;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Copy Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.input = renderPipelineBuilder.readRenderTarget( inputRenderTarget );
            passData.output = renderPipelineBuilder.copyRenderTarget( inputRenderTarget );

            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            ResourceListDesc resListDesc = {};
            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );
            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { outputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { inputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( g_PipelineStateObject );

            cmdList->draw( 3 );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.output;
}

ResHandle_t AddCopyAndDownsampleRenderPass( RenderPipeline* renderPipeline, ResHandle_t inputRenderTarget, const uint32_t outputWidth, const uint32_t outputHeight )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t output;
        ResHandle_t bilinearSampler;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Copy & Downsample Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.input = renderPipelineBuilder.readRenderTarget( inputRenderTarget );

            TextureDescription texDesc = {};
            texDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            texDesc.format = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;
            texDesc.depth = 1;
            texDesc.mipCount = 1;
            texDesc.width = outputWidth;
            texDesc.height = outputHeight;

            passData.output = renderPipelineBuilder.allocateRenderTarget( texDesc, eShaderStage::SHADER_STAGE_PIXEL );

            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            ResourceListDesc resListDesc = {};
            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );
            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { outputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { inputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };

            Viewport vp;
            vp.X = 0;
            vp.Y = 0;
            vp.Width = static_cast<int>( outputWidth );
            vp.Height = static_cast<int>( outputHeight );
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;

            cmdList->setViewport( vp );

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( g_PipelineStateObject );

            cmdList->draw( 3 );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.output;
}

void AddCurrentFrameSaveRenderPass( RenderPipeline* renderPipeline, ResHandle_t inputRenderTarget )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t output;
        ResHandle_t bilinearSampler;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Current Frame Save Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.input = renderPipelineBuilder.readRenderTarget( inputRenderTarget );
            passData.output = renderPipelineBuilder.retrievePersistentRenderTarget( NYA_STRING_HASH( "LastFrameRenderTarget" ) );

            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            ResourceListDesc resListDesc = {};
            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getPersitentRenderTarget( passData.output );
            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { outputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { inputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( g_PipelineStateObject );

            cmdList->draw( 3 );

            renderDevice->destroyRenderPass( renderPass );
        }
    );
}