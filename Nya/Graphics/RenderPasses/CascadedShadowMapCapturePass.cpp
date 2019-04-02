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
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, CommandList* cmdList ) {
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            Buffer* instanceBuffer = renderPipelineResources.getBuffer( passData.instanceBuffer );
            Buffer* vectorDataBuffer = renderPipelineResources.getBuffer( passData.vectorDataBuffer );

#if NYA_DEVBUILD
            Buffer* materialEditorBuffer = renderPipelineResources.getBuffer( passData.materialEditionBuffer );
#endif

            ResourceList resourceList;
            resourceList.resource[0].sampler = bilinearSampler;
            resourceList.resource[1].buffer = instanceBuffer;
            resourceList.resource[2].buffer = vectorDataBuffer;

#if NYA_DEVBUILD
            resourceList.resource[3].buffer = materialEditorBuffer;
#endif


            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );

            // Upload buffer data
            const void* vectorBuffer = renderPipelineResources.getVectorBufferData();
            cmdList->updateBuffer( vectorDataBuffer, vectorBuffer, sizeof( nyaVec4f ) * 1024 );

            RenderPass renderPass;
            renderPass.attachement[0] = { outputTarget, 0, 0 };
            cmdList->clearDepthStencilRenderTarget( outputTarget, 1.0f );

            for ( int i = 0; i < CSM_SLICE_COUNT; i++ ) {
                cmdList->setViewport( { CSM_SHADOW_MAP_DIMENSIONS * i, 0, CSM_SHADOW_MAP_DIMENSIONS, CSM_SHADOW_MAP_DIMENSIONS, 0.0f, 1.0f } );

                const auto& drawCmdBucket = renderPipelineResources.getDrawCmdBucket( DrawCommandKey::LAYER_DEPTH, static_cast<uint8_t>( DrawCommandKey::DEPTH_VIEWPORT_LAYER_CSM0 + i ) );

                InstanceBuffer instanceBufferData;
                instanceBufferData.StartVector = drawCmdBucket.instanceDataStartOffset;
                instanceBufferData.VectorPerInstance = drawCmdBucket.vectorPerInstance;

                cmdList->updateBuffer( instanceBuffer, &instanceBufferData, sizeof( InstanceBuffer ) );

                for ( const auto& drawCmd : drawCmdBucket ) {
                    drawCmd.infos.material->bindDepthOnly( cmdList, renderPass, resourceList );

#if NYA_DEVBUILD
                    const Material::EditorBuffer& matEditBuffer = drawCmd.infos.material->getEditorBuffer();
                    cmdList->updateBuffer( materialEditorBuffer, &matEditBuffer, sizeof( Material::EditorBuffer ) );
#endif

                    cmdList->bindVertexBuffer( drawCmd.infos.vertexBuffer );
                    cmdList->bindIndiceBuffer( drawCmd.infos.indiceBuffer );

                    cmdList->drawInstancedIndexed( drawCmd.infos.indiceBufferCount, drawCmd.infos.instanceCount, drawCmd.infos.indiceBufferOffset );

                    // Update vector buffer offset
                    instanceBufferData.StartVector += ( drawCmd.infos.instanceCount * drawCmdBucket.vectorPerInstance );
                    cmdList->updateBuffer( instanceBuffer, &instanceBufferData, sizeof( InstanceBuffer ) );
                }
            }
        }
    );

    return passData.output;
}
