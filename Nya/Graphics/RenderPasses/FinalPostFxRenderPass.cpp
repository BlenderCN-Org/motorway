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
    psoDesc.resourceListLayout.resources[0] = { 0, SHADER_STAGE_COMPUTE, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_UAV_BUFFER };
    psoDesc.resourceListLayout.resources[1] = { 0, SHADER_STAGE_COMPUTE, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_GENERIC_BUFFER };
    psoDesc.resourceListLayout.resources[2] = { 0, SHADER_STAGE_COMPUTE, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_RENDER_TARGET };
    psoDesc.resourceListLayout.resources[3] = { 1, SHADER_STAGE_COMPUTE, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_RENDER_TARGET };

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
            const Viewport* viewport = renderPipelineResources.getMainViewport();

            Buffer* outputBuffer = renderPipelineResources.getBuffer( passData.output );
            Buffer* autoExposureBuffer = renderPipelineResources.getPersistentBuffer( passData.autoExposureBuffer );
            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );
            RenderTarget* inputBloomTarget = renderPipelineResources.getRenderTarget( passData.inputBloom );

            ResourceList resourceList;
            resourceList.resource[0].buffer = outputBuffer;
            resourceList.resource[1].buffer = autoExposureBuffer;
            resourceList.resource[2].renderTarget = inputTarget;
            resourceList.resource[3].renderTarget = inputBloomTarget;

            cmdList->bindPipelineState( g_PipelineStateObject );
            cmdList->bindResourceList( g_PipelineStateObject, resourceList );

            cmdList->dispatchCompute( static_cast<uint32_t>( viewport->Width ) / 16u, static_cast<uint32_t>( viewport->Height ) / 16u, 1u );
        }
    );

    return passData.output;
}
