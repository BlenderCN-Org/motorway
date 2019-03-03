#include <Shared.h>
#include "FinalPostFxRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/CommandList.h>

static PipelineState*  g_PipelineStateObject = nullptr;

void LoadCachedResourcesFP( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.computeShader = shaderCache->getOrUploadStage( "PostFX/FinalPost", SHADER_STAGE_COMPUTE );

    g_PipelineStateObject = renderDevice->createPipelineState( psoDesc );
}

void FreeCachedResourcesFP( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( g_PipelineStateObject );
}

ResHandle_t AddFinalPostFxRenderPass( RenderPipeline* renderPipeline, ResHandle_t input, ResHandle_t bloomInput )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t inputBloom;

        ResHandle_t output;

        ResHandle_t autoExposureBuffer;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Final PostFx Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.input = renderPipelineBuilder.readRenderTarget( input );
            passData.inputBloom = renderPipelineBuilder.readRenderTarget( bloomInput );
            
            BufferDesc bufferDesc = {};
            bufferDesc.type = BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D;
            bufferDesc.viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;
            bufferDesc.depth = 1;
            bufferDesc.mipCount = 1;

            passData.output = renderPipelineBuilder.allocateBuffer( bufferDesc, eShaderStage::SHADER_STAGE_COMPUTE, RenderPipelineBuilder::eRenderTargetFlags::USE_PIPELINE_DIMENSIONS );
            
            passData.autoExposureBuffer = renderPipelineBuilder.retrievePersistentBuffer( NYA_STRING_HASH( "AutoExposure/ReadBuffer" ) );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Buffer* outputBuffer = renderPipelineResources.getBuffer( passData.output );

            Buffer* autoExposureBuffer = renderPipelineResources.getPersistentBuffer( passData.autoExposureBuffer );

            ResourceListDesc resListDesc = {};
            resListDesc.uavBuffers[0] = { 0, SHADER_STAGE_COMPUTE, outputBuffer };
            resListDesc.buffers[0] = { 0, SHADER_STAGE_COMPUTE, autoExposureBuffer };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );
            RenderTarget* inputBloomTarget = renderPipelineResources.getRenderTarget( passData.inputBloom );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { inputTarget, SHADER_STAGE_COMPUTE, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { inputBloomTarget, SHADER_STAGE_COMPUTE, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( g_PipelineStateObject );
            
            const Viewport* viewport = renderPipelineResources.getMainViewport();
            cmdList->dispatchCompute( viewport->Width / 16u, viewport->Height / 16u, 1u );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.output;
}