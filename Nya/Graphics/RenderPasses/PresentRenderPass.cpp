#include <Shared.h>
#include "PresentRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/CommandList.h>

PipelineState*  g_PipelineStateObject = nullptr;

void LoadCachedResourcesPP( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.vertexShader = shaderCache->getOrUploadStage( "FullscreenTriangle", SHADER_STAGE_VERTEX );
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "PostFX/FinalPost", SHADER_STAGE_PIXEL );
    psoDesc.primitiveTopology = nya::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    psoDesc.rasterizerState.cullMode = nya::rendering::eCullMode::CULL_MODE_NONE;
    psoDesc.depthStencilState.enableDepthTest = false;
    psoDesc.depthStencilState.enableDepthWrite = false;
    psoDesc.blendState.enableBlend = false;
    psoDesc.rasterizerState.useTriangleCCW = false;

    g_PipelineStateObject = renderDevice->createPipelineState( psoDesc );
}

void FreeCachedResourcesPP( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( g_PipelineStateObject );
}

void AddPresentRenderPass( RenderPipeline* renderPipeline, ResHandle_t output )
{
    struct PassData
    {
        ResHandle_t input;
        ResHandle_t bilinearSampler;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Present Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            renderPipelineBuilder.setUncullablePass();

            passData.input = renderPipelineBuilder.readRenderTarget( output );

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
            RenderTarget* outputTarget = renderDevice->getSwapchainBuffer();
            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { outputTarget, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { inputTarget, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( g_PipelineStateObject );
            
            cmdList->draw( 3 );

            renderDevice->destroyRenderPass( renderPass );
        }
    );
}
