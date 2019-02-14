#include <Shared.h>
#include "FinalPostFxRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/CommandList.h>

static PipelineState*  g_PipelineStateObjectWeightedKaris = nullptr;
static PipelineState*  g_PipelineStateObject = nullptr;

void LoadCachedResourcesBP( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.computeShader = shaderCache->getOrUploadStage( "PostFX/DownsampleWeighted+NYA_USE_KARIS_AVERAGE", SHADER_STAGE_COMPUTE );
    g_PipelineStateObjectWeightedKaris = renderDevice->createPipelineState( psoDesc );

    psoDesc.computeShader = shaderCache->getOrUploadStage( "PostFX/DownsampleWeighted", SHADER_STAGE_COMPUTE );
    g_PipelineStateObject = renderDevice->createPipelineState( psoDesc );
}

void FreeCachedResourcesBP( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( g_PipelineStateObjectWeightedKaris );
    renderDevice->destroyPipelineState( g_PipelineStateObject );
}

ResHandle_t AddDownsampleMipRenderPass( RenderPipeline* renderPipeline, ResHandle_t input, const uint32_t inputWidth, const uint32_t inputHeight, const uint32_t downsampleFactor = 2u, const bool useKarisAverage = false )
{
    struct PassData
    {
        ResHandle_t input;
        ResHandle_t output;

        ResHandle_t bilinearSampler;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Downsample Weighted Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            renderPipelineBuilder.setUncullablePass();

            passData.input = ( useKarisAverage ) ? renderPipelineBuilder.readRenderTarget( input ) : renderPipelineBuilder.readBuffer( input );
            
            float downsample = 1.0f / downsampleFactor;

            BufferDesc bufferDesc = {};
            bufferDesc.type = BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D;
            bufferDesc.viewFormat = eImageFormat::IMAGE_FORMAT_R11G11B10_FLOAT;
            bufferDesc.width = static_cast<uint32_t>( inputWidth * downsample );
            bufferDesc.height = static_cast<uint32_t>( inputHeight * downsample );
            bufferDesc.depth = 1;
            bufferDesc.mipCount = 1;

            passData.output = renderPipelineBuilder.allocateBuffer( bufferDesc, eShaderStage::SHADER_STAGE_COMPUTE );

            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Buffer* outputBuffer = renderPipelineResources.getBuffer( passData.output );
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            ResourceListDesc resListDesc = {};
            resListDesc.uavBuffers[0] = { 0, SHADER_STAGE_COMPUTE, outputBuffer };
            resListDesc.samplers[0] = { 0, SHADER_STAGE_COMPUTE, bilinearSampler };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderPassDesc passDesc = {};
            if ( useKarisAverage ) {
                RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );
                
                passDesc.attachements[0] = { inputTarget, SHADER_STAGE_COMPUTE, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };
            } else {
                Buffer* inputBuffer = renderPipelineResources.getBuffer( passData.input );

                passDesc.attachements[0].buffer = inputBuffer;
                passDesc.attachements[0].stageBind = SHADER_STAGE_COMPUTE;
                passDesc.attachements[0].bindMode = RenderPassDesc::READ;
                passDesc.attachements[0].targetState = RenderPassDesc::IS_UAV_TEXTURE;
            }

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( useKarisAverage ? g_PipelineStateObjectWeightedKaris : g_PipelineStateObject );

            cmdList->dispatchCompute( inputWidth / 16u, inputHeight / 16u, 1u );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.output;
}

ResHandle_t AddBlurPyramidRenderPass( RenderPipeline* renderPipeline, ResHandle_t input, uint32_t inputWidth, uint32_t inputHeight )
{
    // Downsample the mipchain
    ResHandle_t mip0RenderTarget = AddDownsampleMipRenderPass( renderPipeline, input, inputWidth, inputHeight, 2, true );
    ResHandle_t mip1RenderTarget = AddDownsampleMipRenderPass( renderPipeline, mip0RenderTarget, inputWidth, inputHeight );
    ResHandle_t mip2RenderTarget = AddDownsampleMipRenderPass( renderPipeline, mip1RenderTarget, inputWidth, inputHeight );
    ResHandle_t mip3RenderTarget = AddDownsampleMipRenderPass( renderPipeline, mip2RenderTarget, inputWidth, inputHeight );
    ResHandle_t mip4RenderTarget = AddDownsampleMipRenderPass( renderPipeline, mip3RenderTarget, inputWidth, inputHeight );

    return mip0RenderTarget;
}
