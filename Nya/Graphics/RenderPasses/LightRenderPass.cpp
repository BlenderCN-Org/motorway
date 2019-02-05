#include <Shared.h>
#include "LightRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>
#include <Graphics/LightGrid.h>

#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

#include <Framework/Material.h>

ResHandle_t AddLightRenderPass( RenderPipeline* renderPipeline, Texture* lightsClusters, Buffer* lightsBuffer, const LightGrid::ClustersInfos& clustersInfos, ResHandle_t output )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t zBuffer;
        ResHandle_t velocityRenderTarget; 
        ResHandle_t thinGBuffer;

        ResHandle_t bilinearSampler;

        ResHandle_t cameraBuffer;
        ResHandle_t instanceBuffer;
        ResHandle_t clustersBuffer;
        ResHandle_t vectorDataBuffer;
    };

    struct InstanceBuffer {
        float StartVector;
        float VectorPerInstance;
        uint32_t __PADDING__[2];
    };

    struct ClustersBuffer {
        glm::vec3 ClustersScale;
        uint32_t __PADDING__;
        glm::vec3 ClustersBias;
        uint32_t __PADDING_2__;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Light Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            // Render Targets
            passData.input = renderPipelineBuilder.readRenderTarget( output );

            TextureDescription velocityRenderTargetDesc = {};
            velocityRenderTargetDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            velocityRenderTargetDesc.format = eImageFormat::IMAGE_FORMAT_R16G16_FLOAT;

            passData.velocityRenderTarget = renderPipelineBuilder.allocateRenderTarget( velocityRenderTargetDesc, RenderPipelineBuilder::USE_PIPELINE_DIMENSIONS );

            TextureDescription zBufferRenderTargetDesc = {};
            zBufferRenderTargetDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            zBufferRenderTargetDesc.format = eImageFormat::IMAGE_FORMAT_R32_TYPELESS;
            zBufferRenderTargetDesc.flags.isDepthResource = 1;

            passData.zBuffer = renderPipelineBuilder.allocateRenderTarget( zBufferRenderTargetDesc, RenderPipelineBuilder::USE_PIPELINE_DIMENSIONS );

            TextureDescription thinGBufferRenderTargetDesc = {};
            thinGBufferRenderTargetDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            thinGBufferRenderTargetDesc.format = eImageFormat::IMAGE_FORMAT_R11G11B10_FLOAT;

            passData.thinGBuffer = renderPipelineBuilder.allocateRenderTarget( thinGBufferRenderTargetDesc, RenderPipelineBuilder::USE_PIPELINE_DIMENSIONS );         
            
            // Fake refcounter increment
            renderPipelineBuilder.readRenderTarget( passData.velocityRenderTarget );
            renderPipelineBuilder.readRenderTarget( passData.thinGBuffer );
            renderPipelineBuilder.readRenderTarget( passData.zBuffer );

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

            BufferDesc vectorDataBufferDesc;
            vectorDataBufferDesc.type = BufferDesc::GENERIC_BUFFER;
            vectorDataBufferDesc.viewFormat = eImageFormat::IMAGE_FORMAT_R32G32B32A32_FLOAT;
            vectorDataBufferDesc.size = sizeof( glm::vec4 ) * 1024;
            vectorDataBufferDesc.stride = 1024;

            passData.vectorDataBuffer = renderPipelineBuilder.allocateBuffer( vectorDataBufferDesc, SHADER_STAGE_VERTEX );

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
            Buffer* vectorDataBuffer = renderPipelineResources.getBuffer( passData.vectorDataBuffer );
             
            ResourceListDesc resListDesc = {};
            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };
            resListDesc.constantBuffers[0] = { 0, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, cameraBuffer };
            resListDesc.constantBuffers[1] = { 1, SHADER_STAGE_VERTEX, instanceBuffer };        
            resListDesc.constantBuffers[2] = { 1, SHADER_STAGE_PIXEL, clustersBuffer };
            resListDesc.constantBuffers[3] = { 2, SHADER_STAGE_PIXEL, lightsBuffer };
            resListDesc.buffers[0] = { 0, SHADER_STAGE_VERTEX, vectorDataBuffer };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.input );
            RenderTarget* zBufferTarget = renderPipelineResources.getRenderTarget( passData.zBuffer );
            RenderTarget* velocityTarget = renderPipelineResources.getRenderTarget( passData.velocityRenderTarget );
            RenderTarget* thinGBufferTarget = renderPipelineResources.getRenderTarget( passData.thinGBuffer );

            // Upload buffer data
            const void* vectorBuffer = renderPipelineResources.getVectorBufferData();
            cmdList->updateBuffer( vectorDataBuffer, vectorBuffer, sizeof( glm::vec4 ) * 1024 );
            
            InstanceBuffer instanceBufferData;
            instanceBufferData.StartVector = 0;
            instanceBufferData.VectorPerInstance = 4;

            cmdList->updateBuffer( instanceBuffer, &instanceBufferData, sizeof( InstanceBuffer ) );

            ClustersBuffer clustersBufferData;
            clustersBufferData.ClustersBias = clustersInfos.Bias;
            clustersBufferData.ClustersScale = clustersInfos.Scale;

            cmdList->updateBuffer( clustersBuffer, &clustersBufferData, sizeof( ClustersBuffer ) );

            const CameraData* cameraData = renderPipelineResources.getMainCamera();
            cmdList->updateBuffer( cameraBuffer, cameraData, sizeof( CameraData ) );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { outputTarget, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { velocityTarget, RenderPassDesc::WRITE, RenderPassDesc::CLEAR_COLOR, { 0, 0, 0, 0 } };
            passDesc.attachements[2] = { thinGBufferTarget, RenderPassDesc::WRITE, RenderPassDesc::CLEAR_COLOR, { 0, 0, 0, 0 } };
            passDesc.attachements[3] = { zBufferTarget, RenderPassDesc::WRITE_DEPTH, RenderPassDesc::CLEAR_DEPTH, { 0, 0, 0, 0 } };

            passDesc.attachements[4].texture = lightsClusters;
            passDesc.attachements[4].bindMode = RenderPassDesc::READ;
            passDesc.attachements[4].targetState = RenderPassDesc::IS_TEXTURE;

            const auto& drawCmdBucket = renderPipelineResources.getDrawCmdBucket( DrawCommandKey::LAYER_WORLD, DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT );
            for ( const auto& drawCmd : drawCmdBucket ) {
                drawCmd.infos.material->bind( cmdList, passDesc );

                RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
                cmdList->useRenderPass( renderPass );

                cmdList->bindVertexBuffer( drawCmd.infos.vertexBuffer );
                cmdList->bindIndiceBuffer( drawCmd.infos.indiceBuffer );

                cmdList->drawIndexed( drawCmd.infos.indiceBufferCount, drawCmd.infos.indiceBufferOffset );

                renderDevice->destroyRenderPass( renderPass );
            }
        }
    );

    return passData.input;
}