#include <Shared.h>
#include "CascadedShadowMapCapturePass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>
#include <Graphics/LightGrid.h>

#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

#include <Framework/Material.h>
#include <Shaders/ShadowMappingShared.h>

ResHandle_t AddCSMCapturePass( RenderPipeline* renderPipeline )
{
    struct PassData  {
        ResHandle_t output;

        ResHandle_t bilinearSampler;

        ResHandle_t cameraBuffer;
        ResHandle_t instanceBuffer;
        ResHandle_t vectorDataBuffer;

#if NYA_DEVBUILD
        // Realtime material edition buffer
        ResHandle_t materialEditionBuffer;
#endif
    };

    struct InstanceBuffer {
        float StartVector;
        float VectorPerInstance;
        uint32_t __PADDING__[2];
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "CSM Capture Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            // Render Targets
            TextureDescription shadowMapRenderTargetDesc = {};
            shadowMapRenderTargetDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            shadowMapRenderTargetDesc.format = eImageFormat::IMAGE_FORMAT_R32_TYPELESS;
            shadowMapRenderTargetDesc.flags.isDepthResource = 1;
            shadowMapRenderTargetDesc.width = CSM_SHADOW_MAP_DIMENSIONS * CSM_SLICE_COUNT;
            shadowMapRenderTargetDesc.height = CSM_SHADOW_MAP_DIMENSIONS;

            passData.output = renderPipelineBuilder.allocateRenderTarget( shadowMapRenderTargetDesc );

            // Buffers
            BufferDesc instanceBufferDesc;
            instanceBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            instanceBufferDesc.size = sizeof( InstanceBuffer );

            passData.instanceBuffer = renderPipelineBuilder.allocateBuffer( instanceBufferDesc, SHADER_STAGE_VERTEX );

            BufferDesc cameraBufferDesc;
            cameraBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            cameraBufferDesc.size = sizeof( CameraData );

            passData.cameraBuffer = renderPipelineBuilder.allocateBuffer( cameraBufferDesc, SHADER_STAGE_VERTEX );

#if NYA_DEVBUILD
            BufferDesc materialBufferDesc;
            materialBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            materialBufferDesc.size = sizeof( Material::EditorBuffer );

            passData.materialEditionBuffer = renderPipelineBuilder.allocateBuffer( materialBufferDesc, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL );
#endif

            BufferDesc vectorDataBufferDesc;
            vectorDataBufferDesc.type = BufferDesc::GENERIC_BUFFER;
            vectorDataBufferDesc.viewFormat = eImageFormat::IMAGE_FORMAT_R32G32B32A32_FLOAT;
            vectorDataBufferDesc.size = sizeof( nyaVec4f ) * 1024;
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
            Buffer* vectorDataBuffer = renderPipelineResources.getBuffer( passData.vectorDataBuffer );
            Buffer* cameraBuffer = renderPipelineResources.getBuffer( passData.cameraBuffer );

#if NYA_DEVBUILD
            Buffer* materialEditorBuffer = renderPipelineResources.getBuffer( passData.materialEditionBuffer );
#endif

            ResourceListDesc resListDesc = {};
            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };
            resListDesc.constantBuffers[0] = { 0, SHADER_STAGE_VERTEX, cameraBuffer };
            resListDesc.constantBuffers[1] = { 1, SHADER_STAGE_VERTEX, instanceBuffer };
            resListDesc.buffers[0] = { 0, SHADER_STAGE_VERTEX, vectorDataBuffer };

#if NYA_DEVBUILD
            resListDesc.constantBuffers[4] = { 3, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, materialEditorBuffer };
#endif

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );

            // Upload buffer data
            const void* vectorBuffer = renderPipelineResources.getVectorBufferData();
            cmdList->updateBuffer( vectorDataBuffer, vectorBuffer, sizeof( nyaVec4f ) * 1024 );

            const CameraData* cameraData = renderPipelineResources.getMainCamera();
            cmdList->updateBuffer( cameraBuffer, cameraData, sizeof( CameraData ) );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { outputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE_DEPTH, RenderPassDesc::CLEAR_DEPTH, { 1, 1, 1, 1 } };
            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );
            renderDevice->destroyRenderPass( renderPass );
            passDesc.attachements[0].targetState = RenderPassDesc::DONT_CARE;

            for ( int i = 0; i < CSM_SLICE_COUNT; i++ ) {
                cmdList->setViewport( { CSM_SHADOW_MAP_DIMENSIONS * i, 0, CSM_SHADOW_MAP_DIMENSIONS, CSM_SHADOW_MAP_DIMENSIONS, 0.0f, 1.0f } );

                const auto& drawCmdBucket = renderPipelineResources.getDrawCmdBucket( DrawCommandKey::LAYER_DEPTH, DrawCommandKey::DEPTH_VIEWPORT_LAYER_CSM0 + i );

                InstanceBuffer instanceBufferData;
                instanceBufferData.StartVector = drawCmdBucket.instanceDataStartOffset;
                instanceBufferData.VectorPerInstance = drawCmdBucket.vectorPerInstance;

                cmdList->updateBuffer( instanceBuffer, &instanceBufferData, sizeof( InstanceBuffer ) );

                for ( const auto& drawCmd : drawCmdBucket ) {
                    drawCmd.infos.material->bindDepthOnly( cmdList, passDesc );

#if NYA_DEVBUILD
                    const Material::EditorBuffer& matEditBuffer = drawCmd.infos.material->getEditorBuffer();
                    cmdList->updateBuffer( materialEditorBuffer, &matEditBuffer, sizeof( Material::EditorBuffer ) );
#endif

                    RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
                    cmdList->useRenderPass( renderPass );

                    cmdList->bindVertexBuffer( drawCmd.infos.vertexBuffer );
                    cmdList->bindIndiceBuffer( drawCmd.infos.indiceBuffer );

                    cmdList->drawInstancedIndexed( drawCmd.infos.indiceBufferCount, drawCmd.infos.instanceCount, drawCmd.infos.indiceBufferOffset );

                    // Update vector buffer offset
                    instanceBufferData.StartVector += ( drawCmd.infos.instanceCount * drawCmdBucket.vectorPerInstance );
                    cmdList->updateBuffer( instanceBuffer, &instanceBufferData, sizeof( InstanceBuffer ) );

                    renderDevice->destroyRenderPass( renderPass );
                }
            }
        }
    );

    return passData.output;
}
