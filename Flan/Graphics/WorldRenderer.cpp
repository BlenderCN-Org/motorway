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
FLAN_ENV_VAR( SSAAMultiplicator, "SSAA Multiplication Factor [1/1.5/2/4/8]", 4.0f, float )

WorldRenderer::WorldRenderer()
    : renderDevice( nullptr )
    , shaderStageManager( nullptr )
    , viewportToRenderCount( 0 )
    , renderPipeline( new RenderPipeline( true ) )
    , textRenderingModule( new TextRenderingModule() )
    , atmosphereRenderingModule( new AtmosphereModule() )
    , autoExposureModule( new AutomaticExposureModule() )
    , lineRenderingModule( new LineRenderingModule() )
    , environmentProbes{ nullptr, nullptr, nullptr }
    , sphereVao( new VertexArrayObject() )
    , rectangleVao( new VertexArrayObject() )
{
    renderInfos.timeDelta = 0.0f;
    renderInfos.worldTime = 0.0f;
    renderInfos.frameNumber = 0;
    renderInfos.backbufferWidth = 0;
    renderInfos.backbufferHeight = 0;

    drawCommands.reserve( 2048 ); 
}

WorldRenderer::~WorldRenderer()
{
    renderDevice = nullptr;
    shaderStageManager = nullptr;
    viewportToRenderCount = 0;
    drawCommands.clear();
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
            Factory<fnPipelineResHandle_t, RenderPipeline*>::tryBuildWithHashcode( renderPass, renderPipeline.get() );
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
}

unsigned int WorldRenderer::getFrameNumber() const
{
    return renderInfos.frameNumber;
}

float WorldRenderer::getTimeDelta() const
{
    return renderInfos.timeDelta;
}

#include "RenderPasses/VMFMapComputeRenderPass.h"
#include "RenderPasses/WriteTextureToDiskPass.h"

void WorldRenderer::saveTexture( RenderTarget* outputTarget, const fnString_t& outputTargetName )
{
    auto renderTarget = renderPipeline->importRenderTarget( outputTarget );
    AddWriteBufferedTextureToDiskPass( renderPipeline.get(), renderDevice, renderTarget, outputTargetName );
}

void WorldRenderer::precomputeVMF( Texture* normalMap, const float roughnessValue, RenderTarget* outputRoughnessMap )
{
    auto resolvedVMF = AddVMFMapComputePass( renderPipeline.get(), normalMap );

    // Build Ouput Texture (merge UAVs into a single mip mapped render target)
    auto roughnessRT = renderPipeline->importRenderTarget( outputRoughnessMap );
    for ( int mipLevel = 0; mipLevel < resolvedVMF.generatedMipCount; mipLevel++ ) {
        AddCopyTextureUAVPass( renderPipeline.get(), false, mipLevel, 0, roughnessRT, resolvedVMF.roughnessMipMaps[mipLevel] );
    }
}

void WorldRenderer::precomputeVMF( Texture* normalMap, Texture* roughnessMap, RenderTarget* outputRoughnessMap )
{
    auto resolvedVMF = AddVMFMapComputePass( renderPipeline.get(), normalMap, roughnessMap );

    // Build Ouput Texture (merge UAVs into a single mip mapped render target)
    auto roughnessRT = renderPipeline->importRenderTarget( outputRoughnessMap );
    for ( int mipLevel = 0; mipLevel < resolvedVMF.generatedMipCount; mipLevel++ ) {
        AddCopyTextureUAVPass( renderPipeline.get(), false, mipLevel, 0, roughnessRT, resolvedVMF.roughnessMipMaps[mipLevel] );
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
    previousFrameDesc.dimension = TextureDescription::DIMENSION_TEXTURE_3D;
    previousFrameDesc.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
    previousFrameDesc.width = 1920;
    previousFrameDesc.height = 1080;
    previousFrameDesc.arraySize = 1;
    previousFrameDesc.mipCount = 1;
    previousFrameDesc.samplerCount = 1;

    previousFrameRenderTarget.reset( new RenderTarget() );
    previousFrameRenderTarget->createAsRenderTarget2D( renderDevice, previousFrameDesc );
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

    return -1u;
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

    return static_cast<uint32_t>( -1u );
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
