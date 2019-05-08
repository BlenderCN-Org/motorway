#include <Shared.h>
#include "VMFMapComputeRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/CommandList.h>

static PipelineState*  g_PipelineStateObject = nullptr;

void LoadCachedResourcesVCP( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.computeShader = shaderCache->getOrUploadStage( "VMFSolver", SHADER_STAGE_COMPUTE );
    psoDesc.resourceListLayout.resources[0] = { 0, SHADER_STAGE_COMPUTE, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
    psoDesc.resourceListLayout.resources[1] = { 1, SHADER_STAGE_COMPUTE, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_TEXTURE };

    g_PipelineStateObject = renderDevice->createPipelineState( psoDesc );
}

void FreeCachedResourcesVCP( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( g_PipelineStateObject );
}

// Computes a compute shader dispatch size given a thread group size, and number of elements to process
uint32_t DispatchSize( uint32_t tgSize, uint32_t numElements )
{
    uint32_t dispatchSize = numElements / tgSize;
    dispatchSize += ( numElements % tgSize > 0 ) ? 1 : 0;
    return dispatchSize;
}

static ResHandle_t AddVMFMapComputePass( RenderPipeline* renderPipeline, Texture* normalMap, const uint32_t normalMapWidth, const uint32_t normalMapHeight, const uint32_t normalMapMipCount, const float roughness )
{
    struct PassBuffer {
        float       inputTexWidth;
        float       inputTexHeight;
        float       outputTexWidth;
        float       outputTexHeight;
        uint32_t    mipLevel;
        float       roughness;
        float       scaleFactor;
    };
    
    struct PassData {
        ResHandle_t passBuffer;
    };

    auto RenderPass = renderPipeline->addRenderPass<PassData>(
        "VMF Solver",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            // Constant Buffer
            BufferDesc bufferDesc;
            bufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            bufferDesc.size = sizeof( PassBuffer );

            passData.passBuffer = renderPipelineBuilder.allocateBuffer( bufferDesc, SHADER_STAGE_COMPUTE );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice ) {
            Buffer* passBuffer = renderPipelineResources.getBuffer( passData.passBuffer );

            ResourceList resourceList;
            resourceList.resource[0].buffer = passBuffer;
            resourceList.resource[1].texture = normalMap;

            renderDevice->updateResourceList( g_PipelineStateObject, resourceList );
            
            PassBuffer passBufferData;
            passBufferData.inputTexWidth = normalMapWidth;
            passBufferData.inputTexHeight = normalMapHeight;
            passBufferData.outputTexWidth = normalMapWidth;
            passBufferData.outputTexHeight = normalMapHeight;
            passBufferData.mipLevel = 0;
            passBufferData.roughness = roughness;
            passBufferData.scaleFactor = std::pow( 10.0f,  0.5f );

            CommandList& cmdList = renderDevice->allocateComputeCommandList();
            {
                cmdList.begin();
                cmdList.bindPipelineState( g_PipelineStateObject );

                auto width = normalMapWidth, height = normalMapHeight;
                for ( unsigned int mipLevel = 0; mipLevel < normalMapMipCount; mipLevel++ ) {
                    passBufferData.mipLevel = mipLevel;
                    passBufferData.outputTexWidth = width;
                    passBufferData.outputTexHeight = height;

                    cmdList.updateBuffer( passBuffer, &passBufferData, sizeof( PassData ) );
                    cmdList.dispatchCompute( DispatchSize( 16, width ), DispatchSize( 16, height ), 1 );

                    width >>= 1;
                    height >>= 1;
                }

                cmdList.end();
            }

            renderDevice->submitCommandList( &cmdList );
        }
    );

    return RenderPass.buffers[0];
}
