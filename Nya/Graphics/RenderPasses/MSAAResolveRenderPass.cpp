#include <Shared.h>
#include "MSAAResolveRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/CommandList.h>

static PipelineState*  g_x2PipelineStateObject = nullptr;
static PipelineState*  g_x4PipelineStateObject = nullptr;
static PipelineState*  g_x8PipelineStateObject = nullptr;
static PipelineState*  g_x2TAAPipelineStateObject = nullptr;
static PipelineState*  g_x4TAAPipelineStateObject = nullptr;
static PipelineState*  g_x8TAAPipelineStateObject = nullptr;

void LoadCachedResourcesMRP( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.vertexShader = shaderCache->getOrUploadStage( "FullscreenTriangle", SHADER_STAGE_VERTEX );
    psoDesc.primitiveTopology = nya::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#define LOAD_PERMUTATION( samplerCount )\
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "MSAAResolve+NYA_MSAA_X" #samplerCount, SHADER_STAGE_PIXEL );\
    g_x##samplerCount##PipelineStateObject = renderDevice->createPipelineState( psoDesc );\
    \
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "MSAAResolve+NYA_MSAA_X" #samplerCount "+NYA_USE_TAA", SHADER_STAGE_PIXEL );\
    g_x##samplerCount##TAAPipelineStateObject = renderDevice->createPipelineState( psoDesc );

    LOAD_PERMUTATION( 2 )
    LOAD_PERMUTATION( 4 )
    LOAD_PERMUTATION( 8 )

#undef LOAD_PERMUTATION
}

void FreeCachedResourcesMRP( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( g_x2PipelineStateObject );
    renderDevice->destroyPipelineState( g_x4PipelineStateObject );
    renderDevice->destroyPipelineState( g_x8PipelineStateObject );
    renderDevice->destroyPipelineState( g_x2TAAPipelineStateObject );
    renderDevice->destroyPipelineState( g_x4TAAPipelineStateObject );
    renderDevice->destroyPipelineState( g_x8TAAPipelineStateObject );
}

PipelineState* GetPermutationPSO( const uint32_t sampleCount, const bool enableTAA )
{
    switch ( sampleCount ) {
    case 2:
        return ( enableTAA ) ? g_x2TAAPipelineStateObject : g_x2PipelineStateObject;
    case 4:
        return ( enableTAA ) ? g_x4TAAPipelineStateObject : g_x4PipelineStateObject;
    case 8:
        return ( enableTAA ) ? g_x8TAAPipelineStateObject : g_x8PipelineStateObject;
    default:
        return nullptr;
    }
}

ResHandle_t AddMSAAResolveRenderPass( RenderPipeline* renderPipeline, ResHandle_t inputRenderTarget, ResHandle_t velocityRenderTarget, ResHandle_t depthRenderTarget, const uint32_t sampleCount, const bool enableTAA )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t inputVelocity;
        ResHandle_t inputDepth;
        ResHandle_t inputLastFrameTarget;

        ResHandle_t output;

        ResHandle_t resolveBuffer;
        ResHandle_t autoExposureBuffer;
    };

    struct ResolveConstantBuffer {
        nyaVec2f    InputTextureDimension;
        float       FilterSize;
        int32_t     SampleRadius;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "MSAA Resolve Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.input = renderPipelineBuilder.readRenderTarget( inputRenderTarget );
            passData.inputVelocity = renderPipelineBuilder.readRenderTarget( velocityRenderTarget );
            passData.inputDepth = renderPipelineBuilder.readRenderTarget( depthRenderTarget );

            passData.output = renderPipelineBuilder.copyRenderTarget( inputRenderTarget, RenderPipelineBuilder::NO_MULTISAMPLE );

            BufferDesc constantBufferDesc = {};
            constantBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            constantBufferDesc.size = sizeof( ResolveConstantBuffer );

            passData.resolveBuffer = renderPipelineBuilder.allocateBuffer( constantBufferDesc, SHADER_STAGE_PIXEL );

            passData.autoExposureBuffer = renderPipelineBuilder.retrievePersistentBuffer( NYA_STRING_HASH( "AutoExposure/ReadBuffer" ) );

            if ( enableTAA ) {
                passData.inputLastFrameTarget = renderPipelineBuilder.retrievePersistentRenderTarget( NYA_STRING_HASH( "LastFrameRenderTarget" ) );
            }
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            // ResourceList
            Buffer* resolveBuffer = renderPipelineResources.getBuffer( passData.resolveBuffer );

            const CameraData* cameraData = renderPipelineResources.getMainCamera();

            ResolveConstantBuffer resolveBufferData;
            resolveBufferData.FilterSize = 2.0f;
            resolveBufferData.SampleRadius = static_cast<int32_t>( ( resolveBufferData.FilterSize / 2.0f ) + 0.499f );
            resolveBufferData.InputTextureDimension = ( cameraData->viewportSize * cameraData->imageQuality );
            cmdList->updateBuffer( resolveBuffer, &resolveBufferData, sizeof( ResolveConstantBuffer ) );

            Buffer* autoExposureBuffer = renderPipelineResources.getPersistentBuffer( passData.autoExposureBuffer );

            ResourceListDesc resListDesc = {};
            resListDesc.constantBuffers[0] = { 0, SHADER_STAGE_PIXEL, resolveBuffer };
            resListDesc.buffers[0] = { 0, SHADER_STAGE_PIXEL, autoExposureBuffer };

            ResourceList& resList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resList );

            // RenderPass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );
            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );
            RenderTarget* depthTarget = renderPipelineResources.getRenderTarget( passData.inputDepth );
            RenderTarget* velocityTarget = renderPipelineResources.getRenderTarget( passData.inputVelocity );

            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { outputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { inputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };
            passDesc.attachements[2] = { velocityTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };
            passDesc.attachements[3] = { depthTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };

            if ( enableTAA ) {
                RenderTarget* lastFrameTarget = renderPipelineResources.getPersitentRenderTarget( passData.inputLastFrameTarget );
                passDesc.attachements[4] = { lastFrameTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };
            }

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( GetPermutationPSO( sampleCount, enableTAA ) );

            cmdList->draw( 3 );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.output;
}
