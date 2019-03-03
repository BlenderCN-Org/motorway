#include <Shared.h>
#include "FinalPostFxRenderPass.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/CommandList.h>

static PipelineState*  g_DownsamplePipelineStateObject = nullptr;
static PipelineState*  g_KarisAveragePipelineStateObject = nullptr;
static PipelineState*  g_UpsamplePipelineStateObject = nullptr;
static PipelineState*  g_BrightPassPipelineStateObject = nullptr;

// Round dimension to a lower even number (avoid padding on GPU and should speed-up (down/up)sampling)
template<typename T>
inline T RoundToEven( const T value )
{
    static_assert( std::is_integral<T>(), "T should be integral (or implement modulo operator)" );
    return ( ( value % 2 == (T)0 ) ? value : ( value - (T)1 ) );
}

void LoadCachedResourcesBP( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.vertexShader = shaderCache->getOrUploadStage( "FullscreenTriangle", SHADER_STAGE_VERTEX );
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "PostFX/Downsample", SHADER_STAGE_PIXEL );
    psoDesc.primitiveTopology = nya::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    g_DownsamplePipelineStateObject = renderDevice->createPipelineState( psoDesc );

    // Mip0 to mip1 pso
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "PostFX/Downsample+NYA_USE_KARIS_AVERAGE", SHADER_STAGE_PIXEL );
    g_KarisAveragePipelineStateObject = renderDevice->createPipelineState( psoDesc );

    // Bright pass pso
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "PostFX/BrightPass", SHADER_STAGE_PIXEL );
    g_BrightPassPipelineStateObject = renderDevice->createPipelineState( psoDesc );

    // Upsample pso
    psoDesc.blendState = {};

    psoDesc.blendState.enableBlend = true;
    psoDesc.blendState.writeMask[0] = true;
    psoDesc.blendState.writeMask[1] = true;
    psoDesc.blendState.writeMask[2] = true;

    psoDesc.blendState.blendConfColor.source = nya::rendering::eBlendSource::BLEND_SOURCE_ONE;
    psoDesc.blendState.blendConfColor.dest = nya::rendering::eBlendSource::BLEND_SOURCE_ONE;
    psoDesc.blendState.blendConfColor.operation = nya::rendering::eBlendOperation::BLEND_OPERATION_ADD;

    psoDesc.pixelShader = shaderCache->getOrUploadStage( "PostFX/Upsample", SHADER_STAGE_PIXEL );
    g_UpsamplePipelineStateObject = renderDevice->createPipelineState( psoDesc );
}

void FreeCachedResourcesBP( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( g_BrightPassPipelineStateObject );
    renderDevice->destroyPipelineState( g_DownsamplePipelineStateObject );
    renderDevice->destroyPipelineState( g_KarisAveragePipelineStateObject );
    renderDevice->destroyPipelineState( g_UpsamplePipelineStateObject );
}

ResHandle_t AddBrightPassRenderPass( RenderPipeline* renderPipeline, ResHandle_t input )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t output;

        ResHandle_t bilinearSampler;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Bright Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.input = renderPipelineBuilder.readRenderTarget( input );
            passData.output = renderPipelineBuilder.copyRenderTarget( input );

            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            RenderTarget* renderTarget = renderPipelineResources.getRenderTarget( passData.output );
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            ResourceListDesc resListDesc = {};
            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );

            // RenderPass
            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { inputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { renderTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( g_BrightPassPipelineStateObject );
            cmdList->draw( 3 );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.output;
}

ResHandle_t AddDownsampleMipRenderPass( RenderPipeline* renderPipeline, ResHandle_t input, const uint32_t inputWidth, const uint32_t inputHeight, const bool useKarisAverage = false,  const uint32_t downsampleFactor = 2u )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t output;

        ResHandle_t bilinearSampler;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Downsample Weighted Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            passData.input = renderPipelineBuilder.readRenderTarget( input );

            const float downsample = ( 1.0f / downsampleFactor );

            TextureDescription texDesc = {};
            texDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            texDesc.format = eImageFormat::IMAGE_FORMAT_R11G11B10_FLOAT;
            texDesc.depth = 1;
            texDesc.mipCount = 1;
            texDesc.width = RoundToEven( static_cast< uint32_t >( inputWidth * downsample ) );
            texDesc.height = RoundToEven( static_cast< uint32_t >( inputHeight * downsample ) );

            passData.output = renderPipelineBuilder.allocateRenderTarget( texDesc, eShaderStage::SHADER_STAGE_PIXEL );

            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            RenderTarget* renderTarget = renderPipelineResources.getRenderTarget( passData.output );
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );

            ResourceListDesc resListDesc = {};
            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );

            // RenderPass
            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { inputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { renderTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };

            const float downsample = ( 1.0f / downsampleFactor );

            cmdList->setViewport( { 0, 0, RoundToEven( static_cast< int >( inputWidth * downsample ) ), RoundToEven( static_cast< int >( inputHeight * downsample ) ), 0.0f, 1.0f } );

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( ( useKarisAverage ) ? g_KarisAveragePipelineStateObject : g_DownsamplePipelineStateObject );
            cmdList->draw( 3 );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.output;
}

ResHandle_t AddUpsampleMipRenderPass( RenderPipeline* renderPipeline, ResHandle_t dst, ResHandle_t src, const uint32_t inputWidth, const uint32_t inputHeight, const float filterRadius = 1.0f, const uint32_t upsampleFactor = 2u )
{
    struct PassData {
        ResHandle_t input;
        ResHandle_t output;

        ResHandle_t bilinearSampler;
        ResHandle_t upsampleInfosBuffer;
    };

    struct UpsampleInfos {
        nyaVec2f    inverseTextureDimensions; // = 1.0f / Input texture 
        float       filterRadius; // NOTE The lower the resolution is, the bigger the radius should be
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Upsample Weighted Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            renderPipelineBuilder.setUncullablePass();

            passData.input = renderPipelineBuilder.readRenderTarget( src );
            passData.output = renderPipelineBuilder.readRenderTarget( dst );

            BufferDesc upsampleBuffer = {};
            upsampleBuffer.type = BufferDesc::CONSTANT_BUFFER;
            upsampleBuffer.size = sizeof( nyaVec4f );

            passData.upsampleInfosBuffer = renderPipelineBuilder.allocateBuffer( upsampleBuffer, eShaderStage::SHADER_STAGE_PIXEL );

            SamplerDesc bilinearSamplerDesc = {};
            bilinearSamplerDesc.addressU = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            RenderTarget* renderTarget = renderPipelineResources.getRenderTarget( passData.output );
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );
           
            Buffer* upsampleBuffer = renderPipelineResources.getBuffer( passData.upsampleInfosBuffer );

            const float inputInverseScaleFactor = ( 1.0f / upsampleFactor );
            const float outputInverseScaleFactor = ( 1.0f / ( upsampleFactor * 2.0f ) );

            UpsampleInfos upsampleInfos;
            upsampleInfos.filterRadius = filterRadius;
            upsampleInfos.inverseTextureDimensions.x = ( 1.0f / ( inputWidth * outputInverseScaleFactor ) );
            upsampleInfos.inverseTextureDimensions.y = ( 1.0f / ( inputHeight * outputInverseScaleFactor ) );

            cmdList->updateBuffer( upsampleBuffer, &upsampleInfos, sizeof( UpsampleInfos ) );

            ResourceListDesc resListDesc = {};
            resListDesc.samplers[0] = { 0, SHADER_STAGE_PIXEL, bilinearSampler };
            resListDesc.constantBuffers[0] = { 0, SHADER_STAGE_PIXEL, upsampleBuffer };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            RenderTarget* inputTarget = renderPipelineResources.getRenderTarget( passData.input );

            // RenderPass
            RenderPassDesc passDesc = {};
            passDesc.attachements[0] = { inputTarget, SHADER_STAGE_PIXEL, RenderPassDesc::READ, RenderPassDesc::DONT_CARE };
            passDesc.attachements[1] = { renderTarget, SHADER_STAGE_PIXEL, RenderPassDesc::WRITE, RenderPassDesc::DONT_CARE };

            cmdList->setViewport( { 0, 0, RoundToEven( static_cast< int >( inputWidth * inputInverseScaleFactor ) ), RoundToEven( static_cast< int >( inputHeight * inputInverseScaleFactor ) ), 0.0f, 1.0f } );

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            cmdList->bindPipelineState( g_UpsamplePipelineStateObject );
            cmdList->draw( 3 );

            renderDevice->destroyRenderPass( renderPass );
        }
    );

    return passData.output;
}

ResHandle_t AddBlurPyramidRenderPass( RenderPipeline* renderPipeline, ResHandle_t input, uint32_t inputWidth, uint32_t inputHeight )
{
    // Do Bright Pass (and implicitly makes a copy of mip0)
    ResHandle_t mip0RenderTarget = AddBrightPassRenderPass( renderPipeline, input );

    // Downsample the mipchain
    ResHandle_t mip1RenderTarget = AddDownsampleMipRenderPass( renderPipeline, mip0RenderTarget, inputWidth, inputHeight, true,  2);
    ResHandle_t mip2RenderTarget = AddDownsampleMipRenderPass( renderPipeline, mip1RenderTarget, inputWidth, inputHeight, false, 4 );
    ResHandle_t mip3RenderTarget = AddDownsampleMipRenderPass( renderPipeline, mip2RenderTarget, inputWidth, inputHeight, false, 8 );
    ResHandle_t mip4RenderTarget = AddDownsampleMipRenderPass( renderPipeline, mip3RenderTarget, inputWidth, inputHeight, false, 16 );
    ResHandle_t mip5RenderTarget = AddDownsampleMipRenderPass( renderPipeline, mip4RenderTarget, inputWidth, inputHeight, false, 32 );

    // Upsample and accumulate mips (using hardware blending)
    ResHandle_t upsampleMip5RenderTarget = mip5RenderTarget; // E' = E
    ResHandle_t upsampleMip4RenderTarget = AddUpsampleMipRenderPass( renderPipeline, mip4RenderTarget, upsampleMip5RenderTarget, inputWidth, inputHeight, 2.00f, 16 );
    ResHandle_t upsampleMip3RenderTarget = AddUpsampleMipRenderPass( renderPipeline, mip3RenderTarget, upsampleMip4RenderTarget, inputWidth, inputHeight, 1.75f, 8 );
    ResHandle_t upsampleMip2RenderTarget = AddUpsampleMipRenderPass( renderPipeline, mip2RenderTarget, upsampleMip3RenderTarget, inputWidth, inputHeight, 1.50f, 4 );
    ResHandle_t upsampleMip1RenderTarget = AddUpsampleMipRenderPass( renderPipeline, mip1RenderTarget, upsampleMip2RenderTarget, inputWidth, inputHeight, 1.25f, 2 );
    ResHandle_t upsampleMip0RenderTarget = AddUpsampleMipRenderPass( renderPipeline, mip0RenderTarget, upsampleMip1RenderTarget, inputWidth, inputHeight, 1.00f, 1 );

    return upsampleMip0RenderTarget;
}