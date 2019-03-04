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

LightPassOutput AddLightRenderPass( RenderPipeline* renderPipeline, const LightGrid::PassData& lightClustersInfos, ResHandle_t sunShadowMap, ResHandle_t output )
{
    struct PassData  {
        ResHandle_t input;
        ResHandle_t zBuffer;
        ResHandle_t velocityRenderTarget;
        ResHandle_t thinGBuffer;
        ResHandle_t sunShadowMap;

        ResHandle_t bilinearSampler;
        ResHandle_t anisotropicSampler;
        ResHandle_t shadowMapSampler;

        ResHandle_t cameraBuffer;
        ResHandle_t instanceBuffer;
        ResHandle_t clustersBuffer;
        ResHandle_t lightsBuffer;
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
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );
            Sampler* anisotropicSampler = renderPipelineResources.getSampler( passData.anisotropicSampler );
            Sampler* shadowMapSampler = renderPipelineResources.getSampler( passData.shadowMapSampler );

            Buffer* instanceBuffer = renderPipelineResources.getBuffer( passData.instanceBuffer );
            Buffer* clustersBuffer = renderPipelineResources.getBuffer( passData.clustersBuffer );
            Buffer* sceneInfosBuffer = renderPipelineResources.getBuffer( passData.sceneInfosBuffer );
            Buffer* cameraBuffer = renderPipelineResources.getBuffer( passData.cameraBuffer );
            Buffer* vectorDataBuffer = renderPipelineResources.getBuffer( passData.vectorDataBuffer );
            Buffer* lightsBuffer = renderPipelineResources.getBuffer( passData.lightsBuffer );

#if NYA_DEVBUILD
            Buffer* materialEditorBuffer = renderPipelineResources.getBuffer( passData.materialEditionBuffer );
#endif

            ResourceListDesc resListDesc = {};
            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };
            resListDesc.samplers[1] = { 1, SHADER_STAGE_PIXEL, anisotropicSampler };
            resListDesc.samplers[2] = { 2, SHADER_STAGE_PIXEL, shadowMapSampler };
            resListDesc.constantBuffers[0] = { 0, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, cameraBuffer };
            resListDesc.constantBuffers[1] = { 1, SHADER_STAGE_VERTEX, instanceBuffer };
            resListDesc.constantBuffers[2] = { 1, SHADER_STAGE_PIXEL, sceneInfosBuffer };
            resListDesc.constantBuffers[3] = { 2, SHADER_STAGE_PIXEL, lightsBuffer };
            resListDesc.buffers[0] = { 0, SHADER_STAGE_VERTEX, vectorDataBuffer };

#if NYA_DEVBUILD
            resListDesc.constantBuffers[4] = { 3, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, materialEditorBuffer };
#endif

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.input );
            RenderTarget* zBufferTarget = renderPipelineResources.getRenderTarget( passData.zBuffer );
            RenderTarget* velocityTarget = renderPipelineResources.getRenderTarget( passData.velocityRenderTarget );
            RenderTarget* thinGBufferTarget = renderPipelineResources.getRenderTarget( passData.thinGBuffer );

            RenderTarget* sunShadowMapTarget = renderPipelineResources.getRenderTarget( passData.sunShadowMap );

            // Upload buffer data
            const void* vectorBuffer = renderPipelineResources.getVectorBufferData();
            cmdList->updateBuffer( vectorDataBuffer, vectorBuffer, sizeof( nyaVec4f ) * 1024 );

            const CameraData* cameraData = renderPipelineResources.getMainCamera();

            nyaVec2f scaledViewportSize = cameraData->viewportSize * cameraData->imageQuality;
            Viewport vp;
            vp.X = 0;
            vp.Y = 0;
            vp.Width = static_cast<int>( scaledViewportSize.x );
            vp.Height = static_cast<int>( scaledViewportSize.y );
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            cmdList->setViewport( vp );

            cmdList->updateBuffer( cameraBuffer, cameraData, sizeof( CameraData ) );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { outputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { velocityTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::CLEAR_COLOR,{ 0, 0, 0, 0 } };
            passDesc.attachements[2] = { thinGBufferTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::CLEAR_COLOR,{ 0, 0, 0, 0 } };
            passDesc.attachements[3] = { zBufferTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE_DEPTH, RenderPassDesc::CLEAR_DEPTH,{ 0, 0, 0, 0 } };

            passDesc.attachements[4].buffer = clustersBuffer;
            passDesc.attachements[4].stageBind = SHADER_STAGE_PIXEL;
            passDesc.attachements[4].bindMode = RenderPassDesc::READ;
            passDesc.attachements[4].targetState = RenderPassDesc::IS_UAV_TEXTURE;

            passDesc.attachements[5] = { sunShadowMapTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };
       
            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );
            renderDevice->destroyRenderPass( renderPass );

            passDesc.attachements[1].targetState = RenderPassDesc::DONT_CARE;
            passDesc.attachements[2].targetState = RenderPassDesc::DONT_CARE;
            passDesc.attachements[3].targetState = RenderPassDesc::DONT_CARE;

            const auto& drawCmdBucket = renderPipelineResources.getDrawCmdBucket( DrawCommandKey::LAYER_WORLD, DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT );

            InstanceBuffer instanceBufferData;
            instanceBufferData.StartVector = drawCmdBucket.instanceDataStartOffset;
            instanceBufferData.VectorPerInstance = drawCmdBucket.vectorPerInstance;

            cmdList->updateBuffer( instanceBuffer, &instanceBufferData, sizeof( InstanceBuffer ) );

            for ( const auto& drawCmd : drawCmdBucket ) {
                drawCmd.infos.material->bind( cmdList, passDesc );

#if NYA_DEVBUILD
                const Material::EditorBuffer& matEditBuffer = drawCmd.infos.material->getEditorBuffer();
                cmdList->updateBuffer( materialEditorBuffer, &matEditBuffer, sizeof( Material::EditorBuffer ) );
#endif

                RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
                cmdList->useRenderPass( renderPass );

                cmdList->bindVertexBuffer( drawCmd.infos.vertexBuffer );
                cmdList->bindIndiceBuffer( drawCmd.infos.indiceBuffer );

                cmdList->drawIndexed( drawCmd.infos.indiceBufferCount, drawCmd.infos.indiceBufferOffset );

                renderDevice->destroyRenderPass( renderPass );
            }
        }
    );

    LightPassOutput passOutput;
    passOutput.lightRenderTarget = passData.input;
    passOutput.velocityRenderTarget = passData.velocityRenderTarget;
    passOutput.depthRenderTarget = passData.zBuffer;

    return passOutput;
}
