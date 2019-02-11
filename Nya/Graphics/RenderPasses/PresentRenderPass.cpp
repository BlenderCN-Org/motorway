#include <Shared.h>
#include "PresentRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/CommandList.h>

PipelineState*  g_PipelineStateObject = nullptr;

void LoadCachedResourcesPP( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.computeShader = shaderCache->getOrUploadStage( "PostFX/FinalPost", SHADER_STAGE_COMPUTE );

    g_PipelineStateObject = renderDevice->createPipelineState( psoDesc );
}

void FreeCachedResourcesPP( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( g_PipelineStateObject );
}

void AddPresentRenderPass( RenderPipeline* renderPipeline, ResHandle_t output )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t output;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Present Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            renderPipelineBuilder.setUncullablePass();

            passData.input = renderPipelineBuilder.readRenderTarget( output );
            
            BufferDesc bufferDesc = {};
            bufferDesc.type = BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D;
            bufferDesc.viewFormat = eImageFormat::IMAGE_FORMAT_R11G11B10_FLOAT;
            bufferDesc.width = 1280;
            bufferDesc.height = 720;
            bufferDesc.depth = 1;
            bufferDesc.mipCount = 1;

            passData.output = renderPipelineBuilder.allocateBuffer( bufferDesc, eShaderStage::SHADER_STAGE_COMPUTE );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Buffer* outputBuffer = renderPipelineResources.getBuffer( passData.input );
            
            ResourceListDesc resListDesc = {};
            resListDesc.uavBuffers[0] = { 0, SHADER_STAGE_COMPUTE, outputBuffer };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { inputTarget, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( g_PipelineStateObject );
            
            const Viewport* viewport = renderPipelineResources.getMainViewport();
            cmdList->dispatchCompute( viewport->Width / 16u, viewport->Height / 16u, 1u );

            renderDevice->destroyRenderPass( renderPass );
        }
    );
}
