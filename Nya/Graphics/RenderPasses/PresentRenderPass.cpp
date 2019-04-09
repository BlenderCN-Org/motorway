#include <Shared.h>
#include "PresentRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/CommandList.h>

static PipelineState*  g_PipelineStateObject = nullptr;

void LoadCachedResourcesPP( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.vertexShader = shaderCache->getOrUploadStage( "FullscreenTriangle", SHADER_STAGE_VERTEX );
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "CopyTexture", SHADER_STAGE_PIXEL );
    psoDesc.primitiveTopology = nya::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    psoDesc.renderPassLayout.attachements[0].stageBind = SHADER_STAGE_PIXEL;
    psoDesc.renderPassLayout.attachements[0].bindMode = RenderPassLayoutDesc::SWAPCHAIN_BUFFER;
    psoDesc.renderPassLayout.attachements[0].targetState = RenderPassLayoutDesc::DONT_CARE;
    psoDesc.renderPassLayout.attachements[0].viewFormat = eImageFormat::IMAGE_FORMAT_B8G8R8A8_UNORM;

    psoDesc.resourceListLayout.resources[0] = { 0u, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER };
    psoDesc.resourceListLayout.resources[1] = { 0u, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_UAV_TEXTURE };

    g_PipelineStateObject = renderDevice->createPipelineState( psoDesc );
}

void FreeCachedResourcesPP( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( g_PipelineStateObject );
}

void AddPresentRenderPass( RenderPipeline* renderPipeline, ResHandle_t inputUAVBuffer )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t bilinearSampler;
    };

    renderPipeline->addRenderPass<PassData>(
        "Present Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            renderPipelineBuilder.setUncullablePass();

            passData.input = renderPipelineBuilder.readBuffer( inputUAVBuffer );

            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice ) {
            RenderTarget* outputTarget = renderDevice->getSwapchainBuffer();
            Buffer* inputBuffer = renderPipelineResources.getBuffer( passData.input );
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            ResourceList resourceList;
            resourceList.resource[0].sampler = bilinearSampler;
            resourceList.resource[1].buffer = inputBuffer;
            renderDevice->updateResourceList( g_PipelineStateObject, resourceList );

            const Viewport* pipelineDimensions = renderPipelineResources.getMainViewport();

            CommandList& cmdList = renderDevice->allocateGraphicsCommandList();
            {
                cmdList.begin();
                cmdList.setViewport( *pipelineDimensions );

                RenderPass renderPass;
                renderPass.attachement[0] = { outputTarget, 0, 0 };

                cmdList.beginRenderPass( g_PipelineStateObject, renderPass );
                {
                    cmdList.bindPipelineState( g_PipelineStateObject );
                    cmdList.draw( 3 );
                }
                cmdList.endRenderPass();

                cmdList.end();
            }

            renderDevice->submitCommandList( &cmdList );
        }
    );
}
