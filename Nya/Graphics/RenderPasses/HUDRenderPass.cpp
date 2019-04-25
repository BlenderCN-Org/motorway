#include <Shared.h>
#include "LightRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>
#include <Graphics/LightGrid.h>

#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

#include <Framework/Material.h>
#include <Core/EnvVarsRegister.h>

#include <Maths/MatrixTransformations.h>

using namespace nya::rendering;

ResHandle_t AddHUDRenderPass( RenderPipeline* renderPipeline, ResHandle_t output )
{
    struct PassData  {
        ResHandle_t input;
        ResHandle_t bilinearSampler;

        ResHandle_t screenBuffer;
        ResHandle_t instanceBuffer;
        ResHandle_t vectorDataBuffer;
        ResHandle_t materialEditionBuffer;
    };

    struct InstanceBuffer {
        float StartVector;
        float VectorPerInstance;
        uint32_t __PADDING__[2];
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "HUD Render Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            renderPipelineBuilder.setUncullablePass();

            // Render Targets
            passData.input = renderPipelineBuilder.readRenderTarget( output );

            // Buffers
            BufferDesc screenBufferDesc;
            screenBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            screenBufferDesc.size = sizeof( nyaMat4x4f );

            passData.screenBuffer = renderPipelineBuilder.allocateBuffer( screenBufferDesc, SHADER_STAGE_VERTEX );

            BufferDesc instanceBufferDesc;
            instanceBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            instanceBufferDesc.size = sizeof( InstanceBuffer );

            passData.instanceBuffer = renderPipelineBuilder.allocateBuffer( instanceBufferDesc, SHADER_STAGE_VERTEX );

            BufferDesc vectorDataBufferDesc;
            vectorDataBufferDesc.type = BufferDesc::GENERIC_BUFFER;
            vectorDataBufferDesc.viewFormat = eImageFormat::IMAGE_FORMAT_R32G32B32A32_FLOAT;
            vectorDataBufferDesc.size = sizeof( nyaVec4f ) * 1024;
            vectorDataBufferDesc.stride = 1024;

            passData.vectorDataBuffer = renderPipelineBuilder.allocateBuffer( vectorDataBufferDesc, SHADER_STAGE_VERTEX );

#if NYA_DEVBUILD
            BufferDesc materialBufferDesc;
            materialBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            materialBufferDesc.size = sizeof( Material::EditorBuffer );

            passData.materialEditionBuffer = renderPipelineBuilder.allocateBuffer( materialBufferDesc, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL );
#endif

            // Misc Resources
            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice ) {
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );
            Buffer* instanceBuffer = renderPipelineResources.getBuffer( passData.instanceBuffer );
            Buffer* screenBuffer = renderPipelineResources.getBuffer( passData.screenBuffer );
            Buffer* vectorDataBuffer = renderPipelineResources.getBuffer( passData.vectorDataBuffer );
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.input );

#if NYA_DEVBUILD
            Buffer* materialEditorBuffer = renderPipelineResources.getBuffer( passData.materialEditionBuffer );
#endif

            ResourceList resourceList;
            resourceList.resource[0].sampler = bilinearSampler;
            resourceList.resource[1].buffer = screenBuffer;
            resourceList.resource[2].buffer = instanceBuffer;
            resourceList.resource[3].buffer = vectorDataBuffer;

#if NYA_DEVBUILD
            resourceList.resource[4].buffer = materialEditorBuffer;
#endif

            CommandList& cmdList = renderDevice->allocateGraphicsCommandList();
            cmdList.begin();

            // Upload buffer data
            const void* vectorBuffer = renderPipelineResources.getVectorBufferData();
            cmdList.updateBuffer( vectorDataBuffer, vectorBuffer, sizeof( nyaVec4f ) * 1024 );

            const CameraData* cameraData = renderPipelineResources.getMainCamera();

            Viewport vp;
            vp.X = 0;
            vp.Y = 0;
            vp.Width = static_cast<int>( cameraData->viewportSize.x );
            vp.Height = static_cast<int>( cameraData->viewportSize.y );
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            cmdList.setViewport( vp );

            nyaMat4x4f orthoMatrix = nya::maths::MakeOrtho( 0.0f, cameraData->viewportSize.x, cameraData->viewportSize.y, 0.0f, -1.0f, 1.0f );

            cmdList.updateBuffer( screenBuffer, &orthoMatrix, sizeof( nyaMat4x4f ) );

            RenderPass renderPass;
            renderPass.attachement[0] = { outputTarget, 0, 0 };

            const auto& drawCmdBucket = renderPipelineResources.getDrawCmdBucket( DrawCommandKey::LAYER_HUD, DrawCommandKey::HUD_VIEWPORT_LAYER_DEFAULT );

            InstanceBuffer instanceBufferData;
            instanceBufferData.StartVector = drawCmdBucket.instanceDataStartOffset;
            instanceBufferData.VectorPerInstance = drawCmdBucket.vectorPerInstance;

            cmdList.updateBuffer( instanceBuffer, &instanceBufferData, sizeof( InstanceBuffer ) );

            for ( const auto& drawCmd : drawCmdBucket ) {
#if NYA_DEVBUILD
                const Material::EditorBuffer& matEditBuffer = drawCmd.infos.material->getEditorBuffer();
                cmdList.updateBuffer( materialEditorBuffer, &matEditBuffer, sizeof( Material::EditorBuffer ) );
#endif

                drawCmd.infos.material->bind( renderDevice, cmdList, renderPass, resourceList );
                {
                    cmdList.bindVertexBuffer( drawCmd.infos.vertexBuffer );
                    cmdList.bindIndiceBuffer( drawCmd.infos.indiceBuffer );

                    cmdList.drawInstancedIndexed( drawCmd.infos.indiceBufferCount, drawCmd.infos.instanceCount, drawCmd.infos.indiceBufferOffset );
                }
                cmdList.endRenderPass();

                // Update vector buffer offset
                instanceBufferData.StartVector += ( drawCmd.infos.instanceCount * drawCmdBucket.vectorPerInstance );
                cmdList.updateBuffer( instanceBuffer, &instanceBufferData, sizeof( InstanceBuffer ) );
            }

            cmdList.end();
            renderDevice->submitCommandList( &cmdList );
        }
    );

    return passData.input;
}
