#include <Shared.h>
#include "LightRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

ResHandle_t AddLightRenderPass( RenderPipeline* renderPipeline, ResHandle_t output )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t velocityRenderTarget;
        ResHandle_t thinGBuffer;

        ResHandle_t bilinearSampler;

        ResHandle_t cameraBuffer;
        ResHandle_t instanceBuffer;
        ResHandle_t clustersBuffer;
    };

    struct InstanceBuffer {
        float StartVector;
        float VectorPerInstance;
        uint32_t __PADDING__[2];
    };

    struct ClustersBuffer {
        float ClustersScale;
        float ClustersBias;
        uint32_t __PADDING__[2];
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Light Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            // Render Targets
            passData.input = renderPipelineBuilder.readRenderTarget( output );

            TextureDescription velocityRenderTargetDesc = {};
            velocityRenderTargetDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            velocityRenderTargetDesc.format = eImageFormat::IMAGE_FORMAT_R16G16_FLOAT;

            passData.velocityRenderTarget = renderPipelineBuilder.allocateRenderTarget( velocityRenderTargetDesc, RenderPipelineBuilder::USE_PIPELINE_DIMENSIONS | RenderPipelineBuilder::USE_PIPELINE_SAMPLER_COUNT );

            TextureDescription thinGBufferRenderTargetDesc = {};
            thinGBufferRenderTargetDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            thinGBufferRenderTargetDesc.format = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

            passData.thinGBuffer = renderPipelineBuilder.allocateRenderTarget( thinGBufferRenderTargetDesc, RenderPipelineBuilder::USE_PIPELINE_DIMENSIONS | RenderPipelineBuilder::USE_PIPELINE_SAMPLER_COUNT );         

            // Buffers
            BufferDesc instanceBufferDesc;
            instanceBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            instanceBufferDesc.size = sizeof( InstanceBuffer );

            passData.instanceBuffer = renderPipelineBuilder.allocateBuffer( instanceBufferDesc, SHADER_STAGE_VERTEX );

            BufferDesc clustersBufferDesc;
            clustersBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            clustersBufferDesc.size = sizeof( ClustersBuffer );

            passData.clustersBuffer = renderPipelineBuilder.allocateBuffer( clustersBufferDesc, SHADER_STAGE_PIXEL );

            BufferDesc cameraBufferDesc;
            cameraBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            cameraBufferDesc.size = sizeof( CameraData );

            passData.cameraBuffer = renderPipelineBuilder.allocateBuffer( cameraBufferDesc, SHADER_STAGE_VERTEX );

            // Misc Resources
            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            Buffer* instanceBuffer = renderPipelineResources.getBuffer( passData.instanceBuffer );
            Buffer* clustersBuffer = renderPipelineResources.getBuffer( passData.clustersBuffer );
            Buffer* cameraBuffer = renderPipelineResources.getBuffer( passData.cameraBuffer );

            ResourceListDesc resListDesc = {};
            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };
            resListDesc.constantBuffers[0] = { 0, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, cameraBuffer };
            resListDesc.constantBuffers[1] = { 1, SHADER_STAGE_VERTEX, instanceBuffer };        
            resListDesc.constantBuffers[2] = { 1, SHADER_STAGE_PIXEL, instanceBuffer };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.input );
            RenderTarget* velocityTarget = renderPipelineResources.getRenderTarget( passData.velocityRenderTarget );
            RenderTarget* thinGBufferTarget = renderPipelineResources.getRenderTarget( passData.thinGBuffer );

            //DrawCmd* drawCmdList = renderPipelineResources.getDrawCmdList();
            //glm::vec4* vectorBuffer = renderPipelineResources.getVectorBufferData();

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { outputTarget, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { velocityTarget, RenderPassDesc::WRITE, RenderPassDesc::CLEAR_COLOR, { 0, 0, 0, 0 } };
            passDesc.attachements[2] = { thinGBufferTarget, RenderPassDesc::WRITE, RenderPassDesc::CLEAR_COLOR, { 0, 0, 0, 0 } };

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.input;
}
