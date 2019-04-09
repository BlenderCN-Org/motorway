#include <Shared.h>
#include "LightRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>
#include <Graphics/LightGrid.h>

#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

#include <Framework/Material.h>
#include <Core/EnvVarsRegister.h>

using namespace nya::rendering;

#define TEXTURE_FILTERING_OPTION_LIST( option )\
    option( BILINEAR )\
    option( TRILINEAR )\
    option( ANISOTROPIC_8 )\
    option( ANISOTROPIC_16 )

NYA_ENV_OPTION_LIST( TextureFiltering, TEXTURE_FILTERING_OPTION_LIST )

NYA_ENV_VAR( TextureFiltering, BILINEAR, eTextureFiltering ) // "Defines texture filtering quality [Bilinear/Trilinear/Anisotropic (8)/Anisotropic (16)]"

LightPassOutput AddLightRenderPass( RenderPipeline* renderPipeline, const LightGrid::PassData& lightClustersInfos, ResHandle_t sunShadowMap, ResHandle_t output, const bool isCapturingProbe )
{
    struct PassData  {
        ResHandle_t input;
        ResHandle_t zBuffer;
        ResHandle_t velocityRenderTarget;
        ResHandle_t thinGBuffer;
        ResHandle_t sunShadowMap;

        ResHandle_t iblDiffuseArray;
        ResHandle_t iblSpecularArray;
        ResHandle_t iblCapturedArray;

        ResHandle_t bilinearSampler;
        ResHandle_t anisotropicSampler;
        ResHandle_t shadowMapSampler;

        ResHandle_t cameraBuffer;
        ResHandle_t instanceBuffer;
        ResHandle_t clustersBuffer;
        ResHandle_t lightsBuffer;
        ResHandle_t itemListBuffer;

        ResHandle_t sceneInfosBuffer;
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
        "Light Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            // Render Targets
            passData.input = renderPipelineBuilder.readRenderTarget( output );
            passData.sunShadowMap = renderPipelineBuilder.readRenderTarget( sunShadowMap );

            passData.iblCapturedArray = renderPipelineBuilder.retrievePersistentRenderTarget( NYA_STRING_HASH( "IBL/CapturedProbesArray" ) );
            passData.iblDiffuseArray = renderPipelineBuilder.retrievePersistentRenderTarget( NYA_STRING_HASH( "IBL/DiffuseProbesArray" ) );
            passData.iblSpecularArray = renderPipelineBuilder.retrievePersistentRenderTarget( NYA_STRING_HASH( "IBL/SpecularProbesArray" ) );

            TextureDescription velocityRenderTargetDesc = {};
            velocityRenderTargetDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            velocityRenderTargetDesc.format = eImageFormat::IMAGE_FORMAT_R16G16_FLOAT;

            passData.velocityRenderTarget = renderPipelineBuilder.allocateRenderTarget( velocityRenderTargetDesc, RenderPipelineBuilder::USE_PIPELINE_DIMENSIONS | RenderPipelineBuilder::USE_PIPELINE_SAMPLER_COUNT );

            TextureDescription zBufferRenderTargetDesc = {};
            zBufferRenderTargetDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            zBufferRenderTargetDesc.format = eImageFormat::IMAGE_FORMAT_R32_TYPELESS;
            zBufferRenderTargetDesc.flags.isDepthResource = 1;

            passData.zBuffer = renderPipelineBuilder.allocateRenderTarget( zBufferRenderTargetDesc, RenderPipelineBuilder::USE_PIPELINE_DIMENSIONS | RenderPipelineBuilder::USE_PIPELINE_SAMPLER_COUNT );

            TextureDescription thinGBufferRenderTargetDesc = {};
            thinGBufferRenderTargetDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            thinGBufferRenderTargetDesc.format = eImageFormat::IMAGE_FORMAT_R11G11B10_FLOAT;

            passData.thinGBuffer = renderPipelineBuilder.allocateRenderTarget( thinGBufferRenderTargetDesc, RenderPipelineBuilder::USE_PIPELINE_DIMENSIONS | RenderPipelineBuilder::USE_PIPELINE_SAMPLER_COUNT );

            // Fake refcounter increment
            renderPipelineBuilder.readRenderTarget( passData.velocityRenderTarget );
            renderPipelineBuilder.readRenderTarget( passData.thinGBuffer );
            renderPipelineBuilder.readRenderTarget( passData.zBuffer );

            // Buffers
            BufferDesc instanceBufferDesc;
            instanceBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            instanceBufferDesc.size = sizeof( InstanceBuffer );

            passData.instanceBuffer = renderPipelineBuilder.allocateBuffer( instanceBufferDesc, SHADER_STAGE_VERTEX );

            passData.clustersBuffer = renderPipelineBuilder.readBuffer( lightClustersInfos.lightsClusters );
            passData.lightsBuffer = renderPipelineBuilder.readBuffer( lightClustersInfos.lightsBuffer );
            passData.sceneInfosBuffer = renderPipelineBuilder.readBuffer( lightClustersInfos.lightsClustersInfosBuffer );
            passData.itemListBuffer = renderPipelineBuilder.readBuffer( lightClustersInfos.itemList );

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

            SamplerDesc materialSamplerDesc = {};
            materialSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            materialSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_WRAP;
            materialSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_WRAP;

            switch ( TextureFiltering ) {
            case eTextureFiltering::ANISOTROPIC_16:
                materialSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_ANISOTROPIC_16;
                break;

            case eTextureFiltering::ANISOTROPIC_8:
                materialSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_ANISOTROPIC_8;
                break;

            case eTextureFiltering::TRILINEAR:
                materialSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_TRILINEAR;
                break;

            default:
            case eTextureFiltering::BILINEAR:
                materialSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;
                break;
            }

            passData.anisotropicSampler = renderPipelineBuilder.allocateSampler( materialSamplerDesc );

            SamplerDesc shadowComparisonSamplerDesc;
            shadowComparisonSamplerDesc.addressU = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_BORDER;
            shadowComparisonSamplerDesc.addressV = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_BORDER;
            shadowComparisonSamplerDesc.addressW = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_BORDER;
            shadowComparisonSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_COMPARISON_TRILINEAR;
            shadowComparisonSamplerDesc.comparisonFunction = eComparisonFunction::COMPARISON_FUNCTION_LEQUAL;

            passData.shadowMapSampler = renderPipelineBuilder.allocateSampler( shadowComparisonSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice ) {
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );
            Sampler* anisotropicSampler = renderPipelineResources.getSampler( passData.anisotropicSampler );
            Sampler* shadowMapSampler = renderPipelineResources.getSampler( passData.shadowMapSampler );

            Buffer* instanceBuffer = renderPipelineResources.getBuffer( passData.instanceBuffer );
            Buffer* clustersBuffer = renderPipelineResources.getBuffer( passData.clustersBuffer );
            Buffer* sceneInfosBuffer = renderPipelineResources.getBuffer( passData.sceneInfosBuffer );
            Buffer* cameraBuffer = renderPipelineResources.getBuffer( passData.cameraBuffer );
            Buffer* vectorDataBuffer = renderPipelineResources.getBuffer( passData.vectorDataBuffer );
            Buffer* lightsBuffer = renderPipelineResources.getBuffer( passData.lightsBuffer );
            Buffer* itemListBuffer = renderPipelineResources.getBuffer( passData.itemListBuffer );

#if NYA_DEVBUILD
            Buffer* materialEditorBuffer = renderPipelineResources.getBuffer( passData.materialEditionBuffer );
#endif

            RenderTarget* sunShadowMapTarget = renderPipelineResources.getRenderTarget( passData.sunShadowMap );

            RenderTarget* iblCapturedArray = renderPipelineResources.getPersitentRenderTarget( passData.iblCapturedArray );
            RenderTarget* iblDiffuseArray = renderPipelineResources.getPersitentRenderTarget( passData.iblDiffuseArray );
            RenderTarget* iblSpecularArray = renderPipelineResources.getPersitentRenderTarget( passData.iblSpecularArray );

            ResourceList resourceList;
            resourceList.resource[0].sampler = bilinearSampler;
            resourceList.resource[1].sampler = anisotropicSampler;
            resourceList.resource[2].sampler = shadowMapSampler;
            resourceList.resource[3].buffer = cameraBuffer;
            resourceList.resource[4].buffer = instanceBuffer;
            resourceList.resource[5].buffer = sceneInfosBuffer;
            resourceList.resource[6].buffer = lightsBuffer;
            resourceList.resource[7].buffer = vectorDataBuffer;
            resourceList.resource[8].buffer = itemListBuffer;

#if NYA_DEVBUILD
            resourceList.resource[9].buffer = materialEditorBuffer;
#endif

            resourceList.resource[10].buffer = clustersBuffer;
            resourceList.resource[11].renderTarget = sunShadowMapTarget;
            resourceList.resource[12].renderTarget = iblDiffuseArray;
            resourceList.resource[13].renderTarget = iblSpecularArray;

            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.input );
            RenderTarget* zBufferTarget = renderPipelineResources.getRenderTarget( passData.zBuffer );
            RenderTarget* velocityTarget = renderPipelineResources.getRenderTarget( passData.velocityRenderTarget );
            RenderTarget* thinGBufferTarget = renderPipelineResources.getRenderTarget( passData.thinGBuffer );

            // Upload buffer data
            const void* vectorBuffer = renderPipelineResources.getVectorBufferData();

            CommandList& cmdList = renderDevice->allocateGraphicsCommandList();
            cmdList.begin();
            cmdList.updateBuffer( vectorDataBuffer, vectorBuffer, sizeof( nyaVec4f ) * 1024 );

            const CameraData* cameraData = renderPipelineResources.getMainCamera();

            nyaVec2f scaledViewportSize = cameraData->viewportSize * cameraData->imageQuality;
            Viewport vp;
            vp.X = 0;
            vp.Y = 0;
            vp.Width = static_cast<int>( scaledViewportSize.x );
            vp.Height = static_cast<int>( scaledViewportSize.y );
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            cmdList.setViewport( vp );

            cmdList.updateBuffer( cameraBuffer, cameraData, sizeof( CameraData ) );

            RenderPass renderPass;
            renderPass.attachement[0] = { outputTarget, 0u, 0u };
            renderPass.attachement[1] = { velocityTarget, 0u, 0u };
            renderPass.attachement[2] = { thinGBufferTarget, 0u, 0u };
            renderPass.attachement[3] = { zBufferTarget, 0u, 0u };
            renderPass.attachement[4] = { sunShadowMapTarget, 0u, 0u };
            renderPass.attachement[5] = { iblDiffuseArray, 0u, 0u };
            renderPass.attachement[6] = { iblSpecularArray, 0u, 0u };

            // Clear renderTargets only once (kinda crap since we can't reuse the renderpass cleaning of D3D12/Vulkan)
            RenderTarget* clearRenderTargets[2] = {
                velocityTarget,
                thinGBufferTarget
            };
            constexpr float CLEAR_VALUE[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            cmdList.clearColorRenderTargets( clearRenderTargets, 2u, CLEAR_VALUE );
            cmdList.clearDepthStencilRenderTarget( zBufferTarget, 0.0f );

            const auto& drawCmdBucket = renderPipelineResources.getDrawCmdBucket( DrawCommandKey::LAYER_WORLD, DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT );

            InstanceBuffer instanceBufferData;
            instanceBufferData.StartVector = drawCmdBucket.instanceDataStartOffset;
            instanceBufferData.VectorPerInstance = drawCmdBucket.vectorPerInstance;

            cmdList.updateBuffer( instanceBuffer, &instanceBufferData, sizeof( InstanceBuffer ) );

            for ( const auto& drawCmd : drawCmdBucket ) {
#if NYA_DEVBUILD
                const Material::EditorBuffer& matEditBuffer = drawCmd.infos.material->getEditorBuffer();
                cmdList.updateBuffer( materialEditorBuffer, &matEditBuffer, sizeof( Material::EditorBuffer ) );
#endif

                if ( !isCapturingProbe ) {
                    drawCmd.infos.material->bind( renderDevice, cmdList, renderPass, resourceList );
                } else {
                    drawCmd.infos.material->bindProbeCapture( renderDevice, cmdList, renderPass, resourceList );
                }
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

    LightPassOutput passOutput;
    passOutput.lightRenderTarget = passData.input;
    passOutput.velocityRenderTarget = passData.velocityRenderTarget;
    passOutput.depthRenderTarget = passData.zBuffer;

    return passOutput;
}
