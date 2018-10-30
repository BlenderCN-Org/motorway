/*
    Project Motorway Source Code
    Copyright (C) 2018 Prévost Baptiste

    This file is part of Project Motorway source code.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <Shared.h>
#include "WorldRenderer.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/Shader.h>
#include <Rendering/Buffer.h>
#include <Rendering/PipelineState.h>
#include <Rendering/Texture.h>
#include <Rendering/Sampler.h>
#include <Rendering/RenderTarget.h>

#include <Core/Factory.h>
#include <Core/Profiler.h>
#include <Core/Maths/Sphere.h>

#include <Core/Allocators/BaseAllocator.h>
#include <Core/Allocators/LinearAllocator.h>

#include "GraphicsAssetManager.h"
#include "ShaderStageManager.h" 
#include "RenderPipeline.h"
#include "TextureSlotIndexes.h"
#include "GraphicsProfiler.h"

#include "RenderModules/TextRenderingModule.h"
#include "RenderModules/AtmosphereModule.h"
#include "RenderModules/AutomaticExposureModule.h"
#include "RenderModules/LineRenderingModule.h"

#include "RenderPasses/PresentPass.h"
#include "RenderPasses/BloomPass.h"
#include "RenderPasses/CompositionPass.h"
#include "RenderPasses/FXAAPass.h"
#include "RenderPasses/WorldDepthPrePass.h"
#include "RenderPasses/DebugWorldPass.h"
#include "RenderPasses/WorldLightPass.h"
#include "RenderPasses/CascadedShadowMapRenderPass.h"
#include "RenderPasses/MSAADepthResolvePass.h"
#include "RenderPasses/MSAAResolvePass.h"
#include "RenderPasses/CopyTexturePass.h"
#include "RenderPasses/UIRenderPass.h"
#include "RenderPasses/EnvironmentProbeConvolutionPass.h"
#include "RenderPasses/VMFMapComputeRenderPass.h"
#include "RenderPasses/WriteTextureToDiskPass.h"

using namespace flan::rendering;

// Env variables options
FLAN_DEV_VAR( ScreenSafeAreaMargin,
              "Defines screen safe area (used mostly by UI elements) [0.0..1.0]",
              0.05f,
              float )

FLAN_DEV_VAR( PrintViewportInfos,
        "Print viewport infos on screen (draw cmds count, viewport id, ...)",
        false,
        bool )

FLAN_DEV_VAR_PERSISTENT( EnvProbeDimension,
                         "Defines environment probe dimension (width and height)",
                         256,
                         uint32_t )

FLAN_ENV_VAR( MSAASamplerCount, "Defines MSAA sampler count [0/2/4/8]", 1, int32_t )
FLAN_ENV_VAR( EnableTemporalAA, "Enables Temporal Antialiasing [false/true]", false, bool )
FLAN_ENV_VAR( EnableFXAA, "Enables FXAA [false/true]", false, bool )
FLAN_ENV_VAR( SSAAMultiplicator, "SSAA Multiplication Factor [1/1.5/2/4/8]", 1.0f, float )

WorldRenderer::WorldRenderer( BaseAllocator* allocator )
    : renderDevice( nullptr )
    , shaderStageManager( nullptr )
    , resourceAllocator( flan::core::allocate<LinearAllocator>( allocator, 64 * 1024 * 1024, allocator->allocate( 64 * 1024 * 1024 ) ) )
    , viewportToRenderCount( 0 )
    , drawCommandsPool( flan::core::allocate<PoolAllocator>( allocator, sizeof( DrawCommand ), 4, sizeof(DrawCommand) * 2048, allocator->allocate( sizeof( DrawCommand ) * 2048 ) ) )
    , renderPipeline( flan::core::allocate<RenderPipeline>( allocator, true ) )
    , textRenderingModule( flan::core::allocate<TextRenderingModule>( allocator ) )
    , atmosphereRenderingModule( flan::core::allocate<AtmosphereModule>( allocator ) )
    , autoExposureModule( flan::core::allocate<AutomaticExposureModule>( allocator ) )
    , lineRenderingModule( flan::core::allocate<LineRenderingModule>( allocator ) )
    , environmentProbes{ nullptr, nullptr, nullptr }
    , sphereVao( flan::core::allocate<VertexArrayObject>( allocator ) )
    , rectangleVao( flan::core::allocate<VertexArrayObject>( allocator ) )
    , boxVao( flan::core::allocate<VertexArrayObject>( allocator ) )
    , circleVao( flan::core::allocate<VertexArrayObject>( allocator ) )
    , coneVao( flan::core::allocate<VertexArrayObject>( allocator ) )
{
    renderInfos.timeDelta = 0.0f;
    renderInfos.worldTime = 0.0f;
    renderInfos.frameNumber = 0;
    renderInfos.backbufferWidth = 0;
    renderInfos.backbufferHeight = 0;
}

WorldRenderer::~WorldRenderer()
{
    renderDevice = nullptr;
    shaderStageManager = nullptr;
    viewportToRenderCount = 0;
}

void WorldRenderer::create( RenderDevice* activeRenderDevice )
{
    renderDevice = activeRenderDevice;

    renderPipeline->create( renderDevice );

#if FLAN_DEVBUILD
    renderPipeline->enableProfiling( renderDevice );
    createPrimitives();
#endif

    // Create shared resources for the renderer (constant buffers, samplers, textures, ... )
    createRenderTargets();
}

void WorldRenderer::destroy()
{
    renderPipeline->destroy( renderDevice );

    // Destroy RenderModules
    textRenderingModule->destroy( renderDevice );
    // atmosphereRenderingModule->destroy( renderDevice );
    // autoExposureModule->destroy( renderDevice );

    previousFrameRenderTarget->destroy( renderDevice );

    environmentProbes[0]->destroy( renderDevice );
    environmentProbes[1]->destroy( renderDevice );
    environmentProbes[2]->destroy( renderDevice );

    sphereVbo->destroy( renderDevice );
    sphereIbo->destroy( renderDevice );
    sphereVao->destroy( renderDevice );
    rectangleVbo->destroy( renderDevice );
    rectangleIbo->destroy( renderDevice );
    rectangleVao->destroy( renderDevice );
}

void WorldRenderer::onFrame( const float interpolatedFrameTime, TaskManager* taskManager )
{
    g_Profiler.beginSection( "WorldRenderer::updateRenderInfos" );
        updateRenderInfos( interpolatedFrameTime );
    g_Profiler.endSection();

    g_Profiler.beginSection( "WorldRenderer::sortDrawCmds" );
        sortDrawCmds();
    g_Profiler.endSection();

    int viewportDrawCmdIdx = 0;
    int viewportTransparentDrawCmdIdx = 0;
    for ( int viewportId = 0; viewportId < viewportToRenderCount; viewportId++ ) {
        g_Profiler.beginSection( "Viewport_" + std::to_string( viewportId ) );
        auto& viewport = viewportsToRender[viewportId];

        renderPipeline->setRendererViewport( viewport );
        renderPipeline->setTimeDelta( renderInfos.timeDelta );

        renderInfos.backbufferWidth = viewport.rendererViewport.Width;
        renderInfos.backbufferHeight = viewport.rendererViewport.Height;

        // Dispatch sorted commands to each layer bucket (for the current viewport index)
        g_Profiler.beginSection( "Viewport_" + std::to_string( viewportId ) + "::DispatchToBuckets" );
        for ( ; viewportDrawCmdIdx < drawCommands.size(); viewportDrawCmdIdx++ ) {
            if ( drawCommands[viewportDrawCmdIdx].key.bitfield.viewportId != viewportId ) {
                break;
            }

            renderPipeline->addToLayerBucket(
                        drawCommands[viewportDrawCmdIdx].key.bitfield.layer,
                        drawCommands[viewportDrawCmdIdx].key.bitfield.viewportLayer,
                        drawCommands[viewportDrawCmdIdx].infos );
        }

        for ( ; viewportTransparentDrawCmdIdx < transparentDrawCommands.size(); viewportTransparentDrawCmdIdx++ ) {
            if ( transparentDrawCommands[viewportTransparentDrawCmdIdx].key.bitfield.viewportId != viewportId ) {
                break;
            }

            renderPipeline->addToLayerBucket(
                transparentDrawCommands[viewportTransparentDrawCmdIdx].key.bitfield.layer,
                transparentDrawCommands[viewportTransparentDrawCmdIdx].key.bitfield.viewportLayer,
                transparentDrawCommands[viewportTransparentDrawCmdIdx].infos );
        }
        g_Profiler.endSection();

#if FLAN_DEVBUILD
        if ( PrintViewportInfos ) {
            std::string camTxt = std::to_string( viewportId )
                + ": worldPos { "
                + std::to_string( viewport.worldViewport.worldPosition.x ) + ", "
                + std::to_string( viewport.worldViewport.worldPosition.y ) + ", "
                + std::to_string( viewport.worldViewport.worldPosition.z )
                + " }, drawCmds "
                + std::to_string( viewportDrawCmdIdx );

            drawDebugText( camTxt, 0.3f, 0.0f, 1.0f - viewportId * 0.1f );
        }
#endif

        g_Profiler.beginSection( "Viewport_" + std::to_string( viewportId ) + "::RenderPassesSubmit" );
        for ( auto renderPass : viewport.renderPasses ) {
            Factory<fnPipelineResHandle_t, RenderPipeline*>::tryBuildWithHashcode( renderPass, renderPipeline );
        }
        g_Profiler.endSection();

        g_Profiler.beginSection( "Viewport_" + std::to_string( viewportId ) + "::RenderPipelineExecution" );
#if FLAN_DEVBUILD
        renderPipeline->executeProfiled( renderDevice, taskManager, shaderStageManager, this );
#else
        renderPipeline->execute( renderDevice, shaderStageManager );
#endif
        g_Profiler.endSection();
        g_Profiler.endSection();
    }

    drawCommands.clear();
    transparentDrawCommands.clear();
    viewportToRenderCount = 0;
}

void WorldRenderer::onResize( const unsigned int width, const unsigned int height )
{
    renderInfos.backbufferWidth = width;
    renderInfos.backbufferHeight = height;
}

RenderPipelineViewport& WorldRenderer::addViewport( const int viewportIndex )
{
    viewportToRenderCount++;
    viewportsToRender[viewportIndex].renderPasses.clear();
    viewportsToRender[viewportIndex].renderPassesArgs.clear();

    return viewportsToRender[viewportIndex];
}

void WorldRenderer::addDrawCommand( DrawCommand&& drawCmd )
{
    if ( drawCmd.key.bitfield.sortOrder == DrawCommandKey::SortOrder::SORT_FRONT_TO_BACK )
        drawCommands.push_back( std::move( drawCmd ) );
    else
        transparentDrawCommands.push_back( std::move( drawCmd ) );
}

void WorldRenderer::drawDebugText( const std::string& text, const float scale, const float x, const float y, const float outlineThickness, const glm::vec4& color, const bool useNormalizedCoordinates )
{
    auto scaleToBackbuffer = renderInfos.backbufferWidth / 1280.0f;

    if ( useNormalizedCoordinates ) {
        auto stringLength = std::min( static_cast<int32_t>( text.size() * 3 ), 240 );

        // Compute safe screen coordinates (avoid rendering stuff outside the viewport)
        auto safeScreenSpacePositionX = 1.0f - ( ( stringLength * scale ) / 240.0f ) - ScreenSafeAreaMargin;
        auto safeScreenSpacePositionY = 1.0f - ( ( scale * 3 / 240.0f ) ) - ScreenSafeAreaMargin;

        float safeX = glm::clamp( x, ScreenSafeAreaMargin, safeScreenSpacePositionX );
        float safeY = glm::clamp( y, ScreenSafeAreaMargin, safeScreenSpacePositionY );

        // Coordinates range is 0..1; a simple product should do the trick
        safeX *= renderInfos.backbufferWidth;
        safeY *= renderInfos.backbufferHeight;

        textRenderingModule->addOutlinedText( text.c_str(), scale * scaleToBackbuffer, safeX, safeY, color, outlineThickness );
    } else {
        textRenderingModule->addOutlinedText( text.c_str(), scale * scaleToBackbuffer, x * renderInfos.backbufferWidth, y * renderInfos.backbufferHeight, color, outlineThickness );
    }
}

void WorldRenderer::drawDebugLine( const glm::vec3& from, const glm::vec3& to, const float thickness, const glm::vec4& color )
{
    lineRenderingModule->addLine( from, to, thickness, color );
}

void WorldRenderer::loadCachedResources( ShaderStageManager* shaderStageManager, GraphicsAssetManager* graphicsAssetManager )
{
    this->shaderStageManager = shaderStageManager;

    // Load RenderModules cached resources (precomputed textures, textures atlases, ...)
    // Register the modules to the factory too
    textRenderingModule->loadCachedResources( renderDevice, graphicsAssetManager );
    atmosphereRenderingModule->loadCachedResources( renderDevice, graphicsAssetManager );
    autoExposureModule->loadCachedResources( renderDevice, graphicsAssetManager );
    lineRenderingModule->loadCachedResources( renderDevice, graphicsAssetManager );

    // Load DFG LUT for standard BRDF
    auto dfgLut = graphicsAssetManager->getTexture( FLAN_STRING( "GameData/Textures/DFG_LUT_Standard.dds" ) );
  
    // Register dynamic render passes
    // TODO Create a dedicated probe module?
    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
                FLAN_STRING_HASH( "ProbeCaptureSavePass" ),
                [=]( RenderPipeline* renderPipeline ) { return addProbeCaptureSavePass( renderPipeline ); } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
                FLAN_STRING_HASH( "ProbeConvolutionPass" ),
                [=]( RenderPipeline* renderPipeline ) { return addProbeConvolutionPass( renderPipeline ); } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
        FLAN_STRING_HASH( "AntiAliasingPassMSAA1" ),
        [=]( RenderPipeline* renderPipeline ) { return addAAPass( renderPipeline, 1, false ); } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
        FLAN_STRING_HASH( "AntiAliasingPassMSAA2" ),
        [=]( RenderPipeline* renderPipeline ) { return addAAPass( renderPipeline, 2, false ); } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
        FLAN_STRING_HASH( "AntiAliasingPassMSAA4" ),
        [=]( RenderPipeline* renderPipeline ) { return addAAPass( renderPipeline, 4, false ); } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
        FLAN_STRING_HASH( "AntiAliasingPassMSAA8" ),
        [=]( RenderPipeline* renderPipeline ) { return addAAPass( renderPipeline, 8, false ); } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
        FLAN_STRING_HASH( "AntiAliasingPassMSAA1TAA" ),
        [=]( RenderPipeline* renderPipeline ) { return addAAPass( renderPipeline, 1, true ); } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
        FLAN_STRING_HASH( "AntiAliasingPassMSAA2TAA" ),
        [=]( RenderPipeline* renderPipeline ) { return addAAPass( renderPipeline, 2, true ); } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
        FLAN_STRING_HASH( "AntiAliasingPassMSAA4TAA" ),
        [=]( RenderPipeline* renderPipeline ) { return addAAPass( renderPipeline, 4, true ); } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
        FLAN_STRING_HASH( "AntiAliasingPassMSAA8TAA" ),
        [=]( RenderPipeline* renderPipeline ) { return addAAPass( renderPipeline, 8, true ); } );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
        FLAN_STRING_HASH( "SSAAResolvePass" ),
        [=]( RenderPipeline* renderPipeline ) { return addSSAAResolvePass( renderPipeline ); } );
    
    // Import Global Resources into the main render pipeline
    brdfInputs.dfgLut = dfgLut;
    brdfInputs.envProbeCapture = environmentProbes[0].get();
    brdfInputs.envProbeDiffuse = environmentProbes[1].get();
    brdfInputs.envProbeSpecular = environmentProbes[2].get();
    renderPipeline->importWellKnownResource( &brdfInputs );

#if FLAN_DEVBUILD
    // Create debug resources

    // Wireframe material
    wireframeMaterial.reset( new Material() );

    PipelineStateDesc descriptor;
    descriptor.vertexStage = shaderStageManager->getOrUploadStage( FLAN_STRING( "Primitive" ), SHADER_STAGE_VERTEX );
    descriptor.pixelStage = shaderStageManager->getOrUploadStage( FLAN_STRING( "Wireframe" ), SHADER_STAGE_PIXEL );
    descriptor.primitiveTopology = flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    RasterizerStateDesc rasterDesc;
    rasterDesc.fillMode = flan::rendering::eFillMode::FILL_MODE_WIREFRAME;
    rasterDesc.cullMode = flan::rendering::eCullMode::CULL_MODE_BACK;
    rasterDesc.useTriangleCCW = false;

    DepthStencilStateDesc depthStencilDesc;
    depthStencilDesc.enableDepthWrite = false;
    depthStencilDesc.enableStencilTest = false;
    depthStencilDesc.enableDepthTest = true;
    depthStencilDesc.depthComparisonFunc = flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_GEQUAL;

    rasterDesc.rebuildStateKey();

    descriptor.rasterizerState = new RasterizerState();
    descriptor.rasterizerState->create( renderDevice, rasterDesc );

    descriptor.depthStencilState = new DepthStencilState();
    descriptor.depthStencilState->create( renderDevice, depthStencilDesc );

    descriptor.inputLayout = {
        { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, false, "POSITION" },
        { 0, IMAGE_FORMAT_R32G32_FLOAT, 0, 0,  0, true, "TEXCOORD" }
    };

    wireframeMaterial->create( renderDevice, shaderStageManager, &descriptor );

    //descriptor.rasterizerState->destroy( renderDevice );
    //descriptor.depthStencilState->destroy( renderDevice );

    //delete descriptor.rasterizerState;
    //delete descriptor.depthStencilState;
#endif
    // TEST Create Fake Material With Global ID0, being loaded at localID 0
    // localID: location in textureArray
    // globalID: splat map texel value, identifying the material (hence the 256 limit)
    auto terrainBaseColor0 = graphicsAssetManager->getTexture( FLAN_STRING( "GameData/Textures/hmapbasecolor2.dds" ) );
    auto& terrainBaseColorDesc = terrainBaseColor0->getDescription();
    for ( unsigned int i = 0; i < terrainBaseColorDesc.mipCount; i++ ) {
        terrainStreamedBaseColor->copySubresource( renderDevice, terrainBaseColor0, i, 0, i, 0 );
    }

    auto terrainBaseColor128 = graphicsAssetManager->getTexture( FLAN_STRING( "GameData/Textures/hmapbasecolor1.dds" ) );
    auto& terrainBaseColor128Desc = terrainBaseColor128->getDescription();
    for ( unsigned int i = 0; i < terrainBaseColor128Desc.mipCount; i++ ) {
        terrainStreamedBaseColor->copySubresource( renderDevice, terrainBaseColor128, i, 0, i, 1 );
    }

    auto terrainBaseColor64 = graphicsAssetManager->getTexture( FLAN_STRING( "GameData/Textures/hmapbasecolor0.dds" ) );
    auto& terrainBaseColor64Desc = terrainBaseColor64->getDescription();
    for ( unsigned int i = 0; i < terrainBaseColor64Desc.mipCount; i++ ) {
        terrainStreamedBaseColor->copySubresource( renderDevice, terrainBaseColor64, i, 0, i, 2 );
    }

    auto terrainNormal0 = graphicsAssetManager->getTexture( FLAN_STRING( "GameData/Textures/hmapnm2.dds" ) );
    auto& terrainNormalDesc = terrainNormal0->getDescription();
    for ( unsigned int i = 0; i < terrainNormalDesc.mipCount; i++ ) {
        terrainStreamedNormal->copySubresource( renderDevice, terrainNormal0, i, 0, i, 0 );
    }

    auto terrainNormal128 = graphicsAssetManager->getTexture( FLAN_STRING( "GameData/Textures/hmapnm1.dds" ) );
    for ( unsigned int i = 0; i < terrainNormalDesc.mipCount; i++ ) {
        terrainStreamedNormal->copySubresource( renderDevice, terrainNormal128, i, 0, i, 1 );
    }

    auto terrainNormal64 = graphicsAssetManager->getTexture( FLAN_STRING( "GameData/Textures/hmapnm0.dds" ) );
    for ( unsigned int i = 0; i < terrainNormalDesc.mipCount; i++ ) {
        terrainStreamedNormal->copySubresource( renderDevice, terrainNormal64, i, 0, i, 2 );
    }

    terrainStreaming.terrainMaterialStreaming[0].terrainSampledSplatIndexes = 0;
    terrainStreaming.terrainMaterialStreaming[0].terrainSamplingParameters = glm::vec4( 0, 0, 96.0f, 96.0f );

    terrainStreaming.terrainMaterialStreaming[64].terrainSampledSplatIndexes = 1;
    terrainStreaming.terrainMaterialStreaming[64].terrainSamplingParameters = glm::vec4( 0, 0, 96.0f, 96.0f );

    terrainStreaming.baseColorStreamed = terrainStreamedBaseColor.get();
    terrainStreaming.normalStreamed = terrainStreamedNormal.get();
    renderPipeline->importWellKnownResource( &terrainStreaming );
}

unsigned int WorldRenderer::getFrameNumber() const
{
    return renderInfos.frameNumber;
}

float WorldRenderer::getTimeDelta() const
{
    return renderInfos.timeDelta;
}

void WorldRenderer::saveTexture( RenderTarget* outputTarget, const fnString_t& outputTargetName )
{
    auto renderTarget = renderPipeline->importRenderTarget( outputTarget );
    AddWriteBufferedTextureToDiskPass( renderPipeline, renderDevice, renderTarget, outputTargetName );
}

void WorldRenderer::precomputeVMF( Texture* normalMap, const float roughnessValue, RenderTarget* outputRoughnessMap )
{
    auto resolvedVMF = AddVMFMapComputePass( renderPipeline, normalMap );

    // Build Ouput Texture (merge UAVs into a single mip mapped render target)
    auto roughnessRT = renderPipeline->importRenderTarget( outputRoughnessMap );
    for ( int mipLevel = 0; mipLevel < resolvedVMF.generatedMipCount; mipLevel++ ) {
        AddCopyTextureUAVPass( renderPipeline, false, mipLevel, 0, roughnessRT, resolvedVMF.roughnessMipMaps[mipLevel] );
    }
}

void WorldRenderer::precomputeVMF( Texture* normalMap, Texture* roughnessMap, RenderTarget* outputRoughnessMap, const bool thightPackedTexture )
{
    auto resolvedVMF = AddVMFMapComputePass( renderPipeline, normalMap, roughnessMap, thightPackedTexture );

    // Build Ouput Texture (merge UAVs into a single mip mapped render target)
    auto roughnessRT = renderPipeline->importRenderTarget( outputRoughnessMap );
    for ( int mipLevel = 0; mipLevel < resolvedVMF.generatedMipCount; mipLevel++ ) {
        AddCopyTextureUAVPass( renderPipeline, false, mipLevel, 0, roughnessRT, resolvedVMF.roughnessMipMaps[mipLevel] );
    }
}


void WorldRenderer::createRenderTargets( void )
{
    TextureDescription envProbeDesc;
    envProbeDesc.dimension = TextureDescription::DIMENSION_TEXTURE_3D;
    envProbeDesc.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
    envProbeDesc.width = EnvProbeDimension;
    envProbeDesc.height = EnvProbeDimension;
    envProbeDesc.depth = 6;
    envProbeDesc.arraySize = MAX_ENVIRONMENT_PROBE_COUNT * envProbeDesc.depth;
    envProbeDesc.mipCount = 1;
    envProbeDesc.samplerCount = 1;
    envProbeDesc.flags.isCubeMap = 1;

    environmentProbes[0].reset( new RenderTarget() );
    environmentProbes[0]->createAsRenderTarget2D( renderDevice, envProbeDesc );

    envProbeDesc.mipCount = flan::rendering::ComputeMipCount( envProbeDesc.width, envProbeDesc.height );
    envProbeDesc.flags.useHardwareMipGen = 1;

    environmentProbes[1].reset( new RenderTarget() );
    environmentProbes[2].reset( new RenderTarget() );

    environmentProbes[1]->createAsRenderTarget2D( renderDevice, envProbeDesc );
    environmentProbes[2]->createAsRenderTarget2D( renderDevice, envProbeDesc );

    TextureDescription previousFrameDesc;
    previousFrameDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    previousFrameDesc.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
    previousFrameDesc.width = static_cast<uint32_t>( 1920 * SSAAMultiplicator );
    previousFrameDesc.height = static_cast<uint32_t>( 1080 * SSAAMultiplicator );
    previousFrameDesc.arraySize = 1;
    previousFrameDesc.mipCount = 1;
    previousFrameDesc.samplerCount = 1;

    previousFrameRenderTarget.reset( new RenderTarget() );
    previousFrameRenderTarget->createAsRenderTarget2D( renderDevice, previousFrameDesc );

    static constexpr int TERRAIN_TEXTURE_DIMENSIONS = 1024;
    TextureDescription terrainTextureStreamingDesc;
    terrainTextureStreamingDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    terrainTextureStreamingDesc.format = IMAGE_FORMAT_BC3_UNORM;
    terrainTextureStreamingDesc.width = TERRAIN_TEXTURE_DIMENSIONS;
    terrainTextureStreamingDesc.height = TERRAIN_TEXTURE_DIMENSIONS;
    terrainTextureStreamingDesc.depth = 1;
    terrainTextureStreamingDesc.arraySize = 32;
    terrainTextureStreamingDesc.mipCount = flan::rendering::ComputeMipCount( TERRAIN_TEXTURE_DIMENSIONS, TERRAIN_TEXTURE_DIMENSIONS );
    terrainTextureStreamingDesc.samplerCount = 1;

    terrainStreamedBaseColor.reset( new Texture() );
    terrainStreamedBaseColor->createAsTexture2D( renderDevice, terrainTextureStreamingDesc );

    terrainTextureStreamingDesc.format = IMAGE_FORMAT_BC3_UNORM;
    terrainStreamedNormal.reset( new Texture() );
    terrainStreamedNormal->createAsTexture2D( renderDevice, terrainTextureStreamingDesc );
}

void WorldRenderer::createPrimitives( void )
{
    {
        Sphere sphere;
        flan::core::CreateSphere( sphere );

        BufferDesc vertexVbo;
        vertexVbo.Type = BufferDesc::VERTEX_BUFFER;
        vertexVbo.Size = sphere.vertices.size() * sizeof( float );
        vertexVbo.Stride = 5 * sizeof( float );

        BufferDesc vertexIbo;
        vertexIbo.Type = BufferDesc::INDICE_BUFFER;
        vertexIbo.Size = sphere.indices.size() * sizeof( uint32_t );
        vertexIbo.Stride = sizeof( uint32_t );

        sphereIndiceCount = static_cast< uint32_t >( sphere.indices.size() );

        sphereVbo.reset( new Buffer() );
        sphereVbo->create( renderDevice, vertexVbo, ( void* )sphere.vertices.data() );

        sphereIbo.reset( new Buffer() );
        sphereIbo->create( renderDevice, vertexIbo, ( void* )sphere.indices.data() );

        sphereVao->create( renderDevice, sphereVbo.get(), sphereIbo.get() );

        VertexLayout_t sphereVertexLayout = {
            { 0, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 0 }, // POSITION
            { 1, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 3 * sizeof( float ) }, // UVMAP0
        };

        sphereVao->setVertexLayout( renderDevice, sphereVertexLayout );
    }
    {
        static constexpr float RECTANGLE_VB_DATA[] = {
            -1.0f, -1.0f, 1.0f,     +0.0f, +0.0f,
            +1.0f, -1.0f, 1.0f,     +1.0f, +0.0f,
            +1.0f, +1.0f, 1.0f,     +1.0f, +1.0f,
            -1.0f, +1.0f, 1.0f,     +0.0f, +1.0f,
        };

        static constexpr uint32_t RECTANGLE_IB_DATA[] = {
            0, 1, 2, 2, 3, 0
        };

        BufferDesc vertexVbo;
        vertexVbo.Type = BufferDesc::VERTEX_BUFFER;
        vertexVbo.Size = sizeof( RECTANGLE_VB_DATA );
        vertexVbo.Stride = 5 * sizeof( float );

        BufferDesc vertexIbo;
        vertexIbo.Type = BufferDesc::INDICE_BUFFER;
        vertexIbo.Size = sizeof( RECTANGLE_IB_DATA );
        vertexIbo.Stride = sizeof( uint32_t );

        rectangleIndiceCount = sizeof( RECTANGLE_IB_DATA ) / sizeof( uint32_t );

        rectangleVbo.reset( new Buffer() );
        rectangleVbo->create( renderDevice, vertexVbo, ( void* )RECTANGLE_VB_DATA );

        rectangleIbo.reset( new Buffer() );
        rectangleIbo->create( renderDevice, vertexIbo, ( void* )RECTANGLE_IB_DATA );

        rectangleVao->create( renderDevice, rectangleVbo.get(), rectangleIbo.get() );

        VertexLayout_t rectangleVertexLayout = {
            { 0, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 0 }, // POSITION
            { 1, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 3 * sizeof( float ) }, // UVMAP0
        };

        rectangleVao->setVertexLayout( renderDevice, rectangleVertexLayout );
    }
    {
        //
        static constexpr int CIRCLE_VERTEX_COUNT = 256;

        std::vector<float> vertexBufferData;
        std::vector<uint32_t> indexBufferData;

        float theta = 0.0f;
        for ( int i = 0; i < CIRCLE_VERTEX_COUNT; i++ ) {
            vertexBufferData.push_back( sin( theta ) );
            vertexBufferData.push_back( 0.0f );
            vertexBufferData.push_back( cos( theta ) );

            vertexBufferData.push_back( 0.0f );
            vertexBufferData.push_back( 0.0f );

            indexBufferData.push_back( i );

            theta += glm::pi<float>() * 4.0f / CIRCLE_VERTEX_COUNT;
        }

        BufferDesc vertexVbo;
        vertexVbo.Type = BufferDesc::VERTEX_BUFFER;
        vertexVbo.Size = ( uint32_t )vertexBufferData.size() * sizeof( float );
        vertexVbo.Stride = 5 * sizeof( float );

        BufferDesc vertexIbo;
        vertexIbo.Type = BufferDesc::INDICE_BUFFER;
        vertexIbo.Size = ( uint32_t )indexBufferData.size() * sizeof( uint32_t );
        vertexIbo.Stride = sizeof( uint32_t );

        circleIndiceCount = static_cast<uint32_t>( indexBufferData.size() );

        circleVbo.reset( new Buffer() );
        circleVbo->create( renderDevice, vertexVbo, ( void* )vertexBufferData.data() );

        circleIbo.reset( new Buffer() );
        circleIbo->create( renderDevice, vertexIbo, ( void* )indexBufferData.data() );

        circleVao->create( renderDevice, circleVbo.get(), circleIbo.get() );

        VertexLayout_t circleVertexLayout = {
            { 0, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 0 }, // POSITION
            { 1, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 3 * sizeof( float ) }, // UVMAP0
        };

        circleVao->setVertexLayout( renderDevice, circleVertexLayout );
    }
    {
        static constexpr float BOX_VB_DATA[] = {
            1.0f, 1.0f, 1.0f, +0.0f, +1.0f,
            -1.0f, 1.0f, 1.0f, +0.0f, -1.0f,
            1.0f, -1.0f, 1.0f, +1.0f, +1.0f,
            -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

            1.0f, 1.0f, 1.0f, +0.0f, +1.0f,
            1.0f, -1.0f, 1.0f, +0.0f, -1.0f,
            1.0f, 1.0f, -1.0f, +1.0f, +1.0f,
            1.0f, -1.0f, -1.0f, -1.0f, -1.0f,

            1.0f, 1.0f, 1.0f,  +0.0f, +1.0f,
            1.0f, 1.0f, -1.0f,  +0.0f, -1.0f,
            -1.0f, 1.0f, 1.0f, +1.0f, +1.0f,
            -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,

            1.0f, 1.0f, -1.0f,  +0.0f, +1.0f,
            1.0f, -1.0f, -1.0f,   +0.0f, -1.0f,
            -1.0f, 1.0f, -1.0f, +1.0f, +1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,

            -1.0f, 1.0f, 1.0f,  +0.0f, +1.0f,
            -1.0f, 1.0f, -1.0f,   +0.0f, -1.0f,
            -1.0f, -1.0f, 1.0f, +1.0f, +1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,

            1.0f, -1.0f, 1.0f,  +0.0f, +1.0f,
            -1.0f, -1.0f, 1.0f,   +0.0f, -1.0f,
            1.0f, -1.0f, -1.0f, +1.0f, +1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        };

        static constexpr uint32_t BOX_IB_DATA[] = {
            0, 1, 2,
            2, 1, 3,

            4, 5, 6,
            6, 5, 7,

            8, 9, 10,
            10, 9, 11,

            12, 13, 14,
            14, 13, 15,

            16, 17, 18,
            18, 17, 19,

            20, 21, 22,
            22, 21, 23,
        };

        BufferDesc vertexVbo;
        vertexVbo.Type = BufferDesc::VERTEX_BUFFER;
        vertexVbo.Size = sizeof( BOX_VB_DATA );
        vertexVbo.Stride = 5 * sizeof( float );

        BufferDesc vertexIbo;
        vertexIbo.Type = BufferDesc::INDICE_BUFFER;
        vertexIbo.Size = sizeof( BOX_IB_DATA );
        vertexIbo.Stride = sizeof( uint32_t );

        boxIndiceCount = static_cast<uint32_t>( vertexIbo.Size / vertexIbo.Stride );

        boxVbo.reset( new Buffer() );
        boxVbo->create( renderDevice, vertexVbo, ( void* )BOX_VB_DATA );

        boxIbo.reset( new Buffer() );
        boxIbo->create( renderDevice, vertexIbo, ( void* )BOX_IB_DATA );

        boxVao->create( renderDevice, boxVbo.get(), boxIbo.get() );

        VertexLayout_t boxVertexLayout = {
            { 0, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 0 }, // POSITION
            { 1, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 3 * sizeof( float ) }, // UVMAP0
        };

        boxVao->setVertexLayout( renderDevice, boxVertexLayout );
    }
    {
        std::vector<float> coneVertices;

        coneVertices.push_back( 0.0f );
        coneVertices.push_back( 0.0f );
        coneVertices.push_back( 0.0f );

        coneVertices.push_back( 0.0f );
        coneVertices.push_back( 0.0f );

        std::vector<uint32_t> coneIndices;
        auto sides = 32.0f;
        auto radius = 1.0f;
        auto height = 1.0f;

        float stepAngle = ( float )( ( 2 * glm::pi<float>() ) / sides );
        for ( int i = 0; i <= sides; i++ ) {
            float r = stepAngle * i;
            float x = ( float )glm::cos( r ) * radius;
            float z = ( float )glm::sin( r ) * radius;

            coneVertices.push_back( x );
            coneVertices.push_back( -1.0f );
            coneVertices.push_back( z );

            coneVertices.push_back( 0.0f );
            coneVertices.push_back( 0.0f );
        }

        for ( int i = 0; i < sides + 1; i++ ) {
            coneIndices.push_back( 0 );
            coneIndices.push_back( i );
            coneIndices.push_back( i + 1 );
        }

        // Generate top and sides
        coneVertices.push_back( 0.0f );
        coneVertices.push_back( height );
        coneVertices.push_back( 0.0f );

        coneVertices.push_back( 0.0f );
        coneVertices.push_back( 0.0f );

        int topInd = ( int )coneIndices.size() - 1;

        for ( int i = 0; i < sides + 1; i++ ) {
            coneIndices.push_back( topInd );
            coneIndices.push_back( i );
            coneIndices.push_back( i + 1 );
        }

        BufferDesc vertexVbo;
        vertexVbo.Type = BufferDesc::VERTEX_BUFFER;
        vertexVbo.Size = coneVertices.size() * sizeof( float );
        vertexVbo.Stride = 5 * sizeof( float );

        BufferDesc vertexIbo;
        vertexIbo.Type = BufferDesc::INDICE_BUFFER;
        vertexIbo.Size = coneIndices.size() * sizeof( uint32_t );
        vertexIbo.Stride = sizeof( uint32_t );

        coneIndiceCount = static_cast<uint32_t>( coneIndices.size() );

        coneVbo.reset( new Buffer() );
        coneVbo->create( renderDevice, vertexVbo, ( void* )coneVertices.data() );

        coneIbo.reset( new Buffer() );
        coneIbo->create( renderDevice, vertexIbo, ( void* )coneIndices.data() );

        coneVao->create( renderDevice, coneVbo.get(), coneIbo.get() );

        VertexLayout_t coneVertexLayout = {
            { 0, VertexLayoutEntry::DIMENSION_XYZ, VertexLayoutEntry::FORMAT_FLOAT, 0 }, // POSITION
            { 1, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 3 * sizeof( float ) }, // UVMAP0
        };

        coneVao->setVertexLayout( renderDevice, coneVertexLayout );
    }
}

void WorldRenderer::sortDrawCmds( void )
{
    std::sort( drawCommands.begin(), drawCommands.end(), &DrawCommand::SortFrontToBack );
    std::sort( transparentDrawCommands.begin(), transparentDrawCommands.end(), &DrawCommand::SortBackToFront );
}

void WorldRenderer::updateRenderInfos( const float interpolatedFrametime )
{
    renderInfos.timeDelta = interpolatedFrametime * 1e-4f;
    renderInfos.worldTime = interpolatedFrametime * 1e-3f;

    renderInfos.frameNumber++;
}

#if FLAN_DEVBUILD
Material* WorldRenderer::getWireframeMaterial() const
{
    return wireframeMaterial.get();
}

VertexArrayObject* WorldRenderer::getSpherePrimitive( uint32_t& indiceCount ) const
{
    indiceCount = sphereIndiceCount;
    return sphereVao.get();
}

VertexArrayObject* WorldRenderer::getRectanglePrimitive( uint32_t& indiceCount ) const
{
    indiceCount = rectangleIndiceCount;
    return rectangleVao.get();
}

VertexArrayObject* WorldRenderer::getCirclePrimitive( uint32_t& indiceCount ) const
{
    indiceCount = circleIndiceCount;
    return circleVao.get();
}

VertexArrayObject* WorldRenderer::getBoxPrimitive( uint32_t& indiceCount ) const
{
    indiceCount = boxIndiceCount;
    return boxVao.get();
}

VertexArrayObject* WorldRenderer::getConePrimitive( uint32_t& indiceCount ) const
{
    indiceCount = coneIndiceCount;
    return coneVao.get();
}
#endif

fnPipelineResHandle_t WorldRenderer::addProbeCaptureSavePass( RenderPipeline* renderPipeline )
{
    renderPipeline->addPipelineSetupPass(
        [&]( RenderPipeline* renderPipeline, RenderPipelineBuilder* renderPipelineBuilder ) {
            const auto captureCmd = static_cast<const EnvProbeCaptureCommand*>( renderPipelineBuilder->getRenderPassArgs( FLAN_STRING_HASH( "ProbeCaptureSavePass" ) ) );

            auto envProbeRT = renderPipeline->importRenderTarget( environmentProbes[0].get() );
            auto faceIndex = captureCmd->CommandInfos.EnvProbeArrayIndex * 6 + captureCmd->CommandInfos.Step;

            AddCopyTexturePass( renderPipeline, false, 0, faceIndex, envProbeRT );
        }
    );

    return -1;
}

fnPipelineResHandle_t WorldRenderer::addProbeConvolutionPass( RenderPipeline* renderPipeline )
{
    renderPipeline->addPipelineSetupPass(
        [&]( RenderPipeline* renderPipeline, RenderPipelineBuilder* renderPipelineBuilder ) {
            auto probeConvolutionCmd = static_cast<const EnvProbeConvolutionCommand*>( renderPipelineBuilder->getRenderPassArgs( FLAN_STRING_HASH( "ProbeConvolutionPass" ) ) );
            auto& cmdInfos = probeConvolutionCmd->CommandInfos;

            auto envProbeRT = renderPipeline->importRenderTarget( environmentProbes[0].get() );

            auto convolutedFaces = AddEnvironmentProbeConvolutionPass( renderPipeline,
                                                                       envProbeRT,
                                                                       cmdInfos.EnvProbeArrayIndex,
                                                                       cmdInfos.Step,
                                                                       cmdInfos.MipIndex );

            auto envProbeDiffuseRT = renderPipeline->importRenderTarget( environmentProbes[1].get() );
            auto envProbeSpecularRT = renderPipeline->importRenderTarget( environmentProbes[2].get() );

            const unsigned int faceIndex = cmdInfos.EnvProbeArrayIndex * 6 + cmdInfos.Step;
            AddCopyTexturePass( renderPipeline, false, cmdInfos.MipIndex, faceIndex, envProbeDiffuseRT, convolutedFaces.StandardDiffuseLD );
            AddCopyTexturePass( renderPipeline, false, cmdInfos.MipIndex, faceIndex, envProbeSpecularRT, convolutedFaces.StandardSpecularLD );
        }
    );

    return static_cast<uint32_t>( -1 );
}

fnPipelineResHandle_t WorldRenderer::addAAPass( RenderPipeline* renderPipeline, const int32_t samplerCount, const bool useTemporalAA )
{
    renderPipeline->addPipelineSetupPass(
        [&]( RenderPipeline* renderPipeline, RenderPipelineBuilder* renderPipelineBuilder ) {
            auto previousFrameRT = renderPipeline->importRenderTarget( previousFrameRenderTarget.get() );
            auto antiAliasedFrame = AddMSAAResolvePass( renderPipeline, samplerCount, useTemporalAA, previousFrameRT );
            
            AddCopyTexturePass( renderPipeline, false, 0, 0, previousFrameRT, antiAliasedFrame );
        }
    );

    return -1;
}

fnPipelineResHandle_t WorldRenderer::addSSAAResolvePass( RenderPipeline* renderPipeline )
{
    renderPipeline->addPipelineSetupPass(
        [&]( RenderPipeline* renderPipeline, RenderPipelineBuilder* renderPipelineBuilder ) {
            auto colorBuffer = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );

            // Downsample Super Sampled render target for the post processing pipeline part
            auto downsampledTarget = AddDownsamplingPass( renderPipeline, colorBuffer, SSAAMultiplicator );
            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ), downsampledTarget );
        }
    );

    return -1;
}
