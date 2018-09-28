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

#include "Shared.h"
#include "RenderableEntityManager.h"
#include "CBufferIndexes.h"

#include <Rendering/Buffer.h>
#include <Rendering/CommandList.h>

#include <Graphics/RenderPipeline.h>

#include <Core/Factory.h>

using namespace flan::framework;

RenderableEntityManager::RenderableEntityManager()
    : DirectionalLightNeedRebuild( false )
    , DirectionalLightsUsed{ false }
    , PointLightNeedRebuild( false )
    , PointLightsUsed{ false }
    , SpotLightsUsed{ false }
    , SpotLightNeedRebuild( false )
    , SphereLightsUsed{ false }
    , SphereLightNeedRebuild( false )
    , DiskLightsUsed{ false }
    , DiskLightNeedRebuild( false )
    , RectangleLightsUsed{ false }
    , RectangleLightNeedRebuild( false )
    , GlobalEnvironmentProbesUsed{ false }
    , GlobalEnvironmentProbeNeedRebuild( false )
    , LocalEnvironmentProbesUsed{ false }
    , LocalEnvironmentProbeNeedRebuild( false )
    , needReupload( false )
    , renderableEntitiesBuffer( nullptr )
{
    renderableEntities.DirectionalLightCount = 0;
    renderableEntities.PointLightCount = 0;
    renderableEntities.SpotLightCount = 0;
    renderableEntities.SphereLightCount = 0;
    renderableEntities.DiskLightCount = 0;
    renderableEntities.RectangleLightCount = 0;
    renderableEntities.LocalEnvironmentProbeCount = 0;
    renderableEntities.GlobalEnvironmentProbeCount = 0;
}

void RenderableEntityManager::create( RenderDevice* renderDevice )
{
    // Buffer Description
    BufferDesc renderableEntitiesBufferDesc;
    renderableEntitiesBufferDesc.Type = BufferDesc::CONSTANT_BUFFER;
    renderableEntitiesBufferDesc.Size = sizeof( renderableEntities );

    // Try to create the buffer
    renderableEntitiesBuffer.reset( new Buffer() );
    renderableEntitiesBuffer->create( renderDevice, renderableEntitiesBufferDesc );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( 
        FLAN_STRING_HASH( "LightCullingPass" ), 
        [=]( RenderPipeline* renderPipeline ) { 
            return addLightCullingPass( renderPipeline, false ); 
        } 
    );

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent(
        FLAN_STRING_HASH( "LightCullingMSAAPass" ),
        [=]( RenderPipeline* renderPipeline ) {
            return addLightCullingPass( renderPipeline, true );
        }
    );
}

void RenderableEntityManager::rebuildBuffer( RenderDevice* renderDevice )
{
    // Check buffer dirtiness before start updating it
    if ( !needReupload ) {
        return;
    }

#define PA_UPDATE_ENTITY_ARRAY( type, enumType, dataGetter )\
{\
    if ( type##NeedRebuild ) {\
        int gpuIterator = 0;\
        for ( int cpuIterator = 0; cpuIterator < MAX_##enumType##_COUNT; cpuIterator++ ) {\
            if ( type##sUsed[cpuIterator] ) {\
                renderableEntities.type##s[gpuIterator] = type##s[cpuIterator]dataGetter;\
                ++gpuIterator;\
                \
                if ( gpuIterator == renderableEntities.type##Count ) {\
                    break;\
                }\
            }\
        }\
        type##NeedRebuild = false;\
    }\
}

    PA_UPDATE_ENTITY_ARRAY( DirectionalLight, DIRECTIONAL_LIGHT, .getLightData() );
    PA_UPDATE_ENTITY_ARRAY( PointLight, POINT_LIGHT, .getLightData() );
    PA_UPDATE_ENTITY_ARRAY( SpotLight, SPOT_LIGHT, .getLightData() );
    PA_UPDATE_ENTITY_ARRAY( SphereLight, SPHERE_LIGHT, .getLightData() );
    PA_UPDATE_ENTITY_ARRAY( DiskLight, DISC_LIGHT, .getLightData() );
    PA_UPDATE_ENTITY_ARRAY( RectangleLight, RECTANGLE_LIGHT, .getLightData() );

    // NOTE Probe does not use a getter (the last arg of the macro should be empty)
    PA_UPDATE_ENTITY_ARRAY( GlobalEnvironmentProbe, GLOBAL_ENVIRONMENT_PROBE, );
    PA_UPDATE_ENTITY_ARRAY( LocalEnvironmentProbe, LOCAL_ENVIRONMENT_PROBE, );

#undef PA_UPDATE_ENTITY_ARRAY

    renderableEntitiesBuffer->update( renderDevice, &renderableEntities, sizeof( renderableEntities ) );

    // Update dirty flag
    needReupload = false;
}

void RenderableEntityManager::clear()
{
    memset( &renderableEntities, 0, sizeof( renderableEntities ) );

    memset( &DirectionalLightsUsed, false, MAX_DIRECTIONAL_LIGHT_COUNT * sizeof( bool ) );
    memset( &PointLightsUsed, false, MAX_POINT_LIGHT_COUNT * sizeof( bool ) );
    memset( &SpotLightsUsed, false, MAX_SPOT_LIGHT_COUNT * sizeof( bool ) );
    memset( &SphereLightsUsed, false, MAX_SPHERE_LIGHT_COUNT * sizeof( bool ) );
    memset( &DiskLightsUsed, false, MAX_DISC_LIGHT_COUNT * sizeof( bool ) );
    memset( &RectangleLightsUsed, false, MAX_RECTANGLE_LIGHT_COUNT * sizeof( bool ) );
    memset( &GlobalEnvironmentProbesUsed, false, MAX_GLOBAL_ENVIRONMENT_PROBE_COUNT * sizeof( bool ) );
    memset( &LocalEnvironmentProbesUsed, false, MAX_LOCAL_ENVIRONMENT_PROBE_COUNT * sizeof( bool ) );

    needReupload = true;
}

EnvironmentProbe* RenderableEntityManager::createLocalEnvironmentProbe()
{
    if ( renderableEntities.LocalEnvironmentProbeCount >= MAX_LOCAL_ENVIRONMENT_PROBE_COUNT ) {
        FLAN_CERR << "Local probe buffer is full!" << std::endl;
        return nullptr;
    }

    int cpuIndex = 0;
    for ( ; cpuIndex < MAX_LOCAL_ENVIRONMENT_PROBE_COUNT; cpuIndex++ ) {
        if ( !LocalEnvironmentProbesUsed[cpuIndex] ) {
            LocalEnvironmentProbesUsed[cpuIndex] = true;
            break;
        }
    }

    LocalEnvironmentProbes[cpuIndex] = {};
    LocalEnvironmentProbes[cpuIndex].RenderKey = BuildRenderKey( RENDERABLE_TYPE_LOCAL_ENVIRONMENT_PROBE, cpuIndex );
    LocalEnvironmentProbes[cpuIndex].IsCaptured = false;
    LocalEnvironmentProbes[cpuIndex].ProbeIndex = renderableEntities.LocalEnvironmentProbeCount + MAX_GLOBAL_ENVIRONMENT_PROBE_COUNT;
    LocalEnvironmentProbes[cpuIndex].Sphere.center = glm::vec3( 0.0f );
    LocalEnvironmentProbes[cpuIndex].Sphere.radius = 0.0f;

    renderableEntities.LocalEnvironmentProbeCount++;

    return &LocalEnvironmentProbes[cpuIndex];
}

EnvironmentProbe* RenderableEntityManager::createGlobalEnvironmentProbe()
{
    if ( renderableEntities.GlobalEnvironmentProbeCount >= MAX_GLOBAL_ENVIRONMENT_PROBE_COUNT ) {
        FLAN_CERR << "Global probe buffer is full!" << std::endl;
        return nullptr;
    }

    int cpuIndex = 0;
    for ( ; cpuIndex < MAX_GLOBAL_ENVIRONMENT_PROBE_COUNT; cpuIndex++ ) {
        if ( !GlobalEnvironmentProbesUsed[cpuIndex] ) {
            GlobalEnvironmentProbesUsed[cpuIndex] = true;
            break;
        }
    }

    GlobalEnvironmentProbes[cpuIndex] = {};

    auto& probe = GlobalEnvironmentProbes[cpuIndex];
    probe.RenderKey = BuildRenderKey( RENDERABLE_TYPE_GLOBAL_ENVIRONMENT_PROBE, cpuIndex );
    probe.IsCaptured = false;
    probe.ProbeIndex = renderableEntities.GlobalEnvironmentProbeCount;
    probe.IsFallbackProbe = true;

    renderableEntities.GlobalEnvironmentProbeCount++;

    return &probe;
}

void RenderableEntityManager::updateEntity( const fnRenderKey_t renderKey )
{
#define PA_FLAG_ENTITY_LIST_TO_RELOAD( type, enumType )\
    case RENDERABLE_TYPE_##enumType:\
    if ( entityIndex > renderableEntities.type##Count ) {\
        break;\
    }\
    \
    type##NeedRebuild = true;\
break;\

    const auto entityType = ( flan::framework::eRenderableType )( renderKey & 0xFFFFFFFF );
    const uint32_t entityIndex = ( uint32_t )( ( renderKey & 0xFFFFFFFF00000000 ) >> 32 );

    switch ( entityType ) {
    PA_FLAG_ENTITY_LIST_TO_RELOAD( DirectionalLight, DIRECTIONAL_LIGHT )
    PA_FLAG_ENTITY_LIST_TO_RELOAD( PointLight, POINT_LIGHT )
    PA_FLAG_ENTITY_LIST_TO_RELOAD( SpotLight, SPOT_LIGHT )
    PA_FLAG_ENTITY_LIST_TO_RELOAD( SphereLight, SPHERE_LIGHT )
    PA_FLAG_ENTITY_LIST_TO_RELOAD( DiskLight, DISC_LIGHT );
    PA_FLAG_ENTITY_LIST_TO_RELOAD( RectangleLight, RECTANGLE_LIGHT );
    PA_FLAG_ENTITY_LIST_TO_RELOAD( GlobalEnvironmentProbe, GLOBAL_ENVIRONMENT_PROBE );
    PA_FLAG_ENTITY_LIST_TO_RELOAD( LocalEnvironmentProbe, LOCAL_ENVIRONMENT_PROBE );

    default:
        return;
    }

#undef PA_FLAG_ENTITY_LIST_TO_RELOAD

    needReupload = true;
}

void RenderableEntityManager::removeEntity( const fnRenderKey_t renderKey )
{
    const eRenderableType renderType = ( eRenderableType )( renderKey & 0xFFFFFFFF );
    const uint32_t lightIndex = ( uint32_t )( ( renderKey & 0xFFFFFFFF00000000 ) >> 32 );

#define PA_REMOVE_ENTITY_CASE( type, enumType )\
    case RENDERABLE_TYPE_##enumType:\
    if ( lightIndex > renderableEntities.type##Count ) {\
        break;\
    }\
    \
    type##sUsed[lightIndex] = false;\
    renderableEntities.type##Count--;\
    break;

    switch ( renderType ) {
    PA_REMOVE_ENTITY_CASE( DirectionalLight, DIRECTIONAL_LIGHT )
    PA_REMOVE_ENTITY_CASE( PointLight, POINT_LIGHT )
    PA_REMOVE_ENTITY_CASE( SpotLight, SPOT_LIGHT )
    PA_REMOVE_ENTITY_CASE( SphereLight, SPHERE_LIGHT )
    PA_REMOVE_ENTITY_CASE( DiskLight, DISC_LIGHT );
    PA_REMOVE_ENTITY_CASE( RectangleLight, RECTANGLE_LIGHT );
    PA_REMOVE_ENTITY_CASE( GlobalEnvironmentProbe, GLOBAL_ENVIRONMENT_PROBE );
    PA_REMOVE_ENTITY_CASE( LocalEnvironmentProbe, LOCAL_ENVIRONMENT_PROBE );
  
    default:
        return;
    }

#undef PA_REMOVE_ENTITY_CASE

    needReupload = true;
}

fnPipelineMutableResHandle_t RenderableEntityManager::addLightCullingPass( RenderPipeline* renderPipeline, bool enableMSAA )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Forward Plus Light Culling",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            // Pipeline State
            RenderPassPipelineStateDesc passPipelineState = {};

            if ( enableMSAA ) {
                passPipelineState.hashcode = FLAN_STRING_HASH( "FPlus Light Culling MSAA" );
                passPipelineState.computeStage = FLAN_STRING( "LightCullingMSAA" );
            } else {
                passPipelineState.hashcode = FLAN_STRING_HASH( "FPlus Light Culling" );
                passPipelineState.computeStage = FLAN_STRING( "LightCulling" );
            }
            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( passPipelineState );

            // Read Depth Buffer
            passData.input[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainDepthRT" ) );

            const auto& viewportSize = renderPipelineBuilder->getActiveViewportGeometry();

            // Compute light buffer size (based on backbuffer size and tuning parameters)
            unsigned int tileCountX = static_cast<unsigned int>( ( viewportSize.Width + TILE_RES - 1 ) / static_cast<float>( TILE_RES ) );
            unsigned int tileCountY = static_cast<unsigned int>( ( viewportSize.Height + TILE_RES - 1 ) / static_cast<float>( TILE_RES ) );

            unsigned int totalTileCount = tileCountX * tileCountY;

            // Backbuffer height should be clamped to avoid unexpected results (see AMD Forward Plus sample)
            unsigned int clampedHeight = ( viewportSize.Height < 720 ) ? viewportSize.Height : 720;

            unsigned int maxNumLightPerTile = ( MAX_NUM_LIGHTS_PER_TILE - ( ADJUSTMENT_MULTIPLIER * ( clampedHeight / 120 ) ) );

            const unsigned int lightIndexBufferCapacity = maxNumLightPerTile * totalTileCount;
            const std::size_t lightIndexBufferSize = sizeof( unsigned int ) * lightIndexBufferCapacity;

            // Light Indexes UAV Buffer
            BufferDesc passBuffer;
            passBuffer.Type = BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER;
            passBuffer.ViewFormat = IMAGE_FORMAT_R32_UINT;
            passBuffer.Size = lightIndexBufferSize;
            passBuffer.Stride = static_cast<uint32_t>( lightIndexBufferSize / sizeof( uint32_t ) );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( passBuffer );

            // Camera Buffer
            BufferDesc cameraBuffer;
            cameraBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            cameraBuffer.Size = sizeof( Camera::Data );

            passData.buffers[1] = renderPipelineBuilder->allocateBuffer( cameraBuffer );

            // Viewport Buffer
            BufferDesc viewportBuffer;
            viewportBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            viewportBuffer.Size = sizeof( glm::uvec4 );

            passData.buffers[2] = renderPipelineBuilder->allocateBuffer( viewportBuffer );

            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "ForwardPlusLightIndexBuffer" ), passData.buffers[0] );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind light index buffer
            auto depthBuffer = renderPipelineResources->getRenderTarget( passData.input[0] );
            depthBuffer->bind( cmdList, 1, SHADER_STAGE_COMPUTE );

            // Bind Resources
            auto lightIndexBuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            lightIndexBuffer->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            // Bind Camera Buffer
            auto& passCamera = renderPipelineResources->getActiveCamera();
            auto cameraBuffer = renderPipelineResources->getBuffer( passData.buffers[1] );
            cameraBuffer->updateAsynchronous( cmdList, &passCamera, sizeof( Camera::Data ) );
            cameraBuffer->bind( cmdList, 0, SHADER_STAGE_COMPUTE );

            // Bind Camera Buffer
            glm::uvec2 rtDimensions;

            auto& activeViewport = renderPipelineResources->getActiveViewportGeometry();

            rtDimensions.x = activeViewport.Width;
            rtDimensions.y = activeViewport.Height;

            auto bufferData = renderPipelineResources->getBuffer( passData.buffers[2] );
            bufferData->updateAsynchronous( cmdList, &rtDimensions, sizeof( glm::uvec2 ) );
            bufferData->bind( cmdList, 2, SHADER_STAGE_COMPUTE );

            renderableEntitiesBuffer->bind( cmdList, CBUFFER_INDEX_LIGHTBUFFER, SHADER_STAGE_COMPUTE );

            // Retrieve tile count
            const unsigned int tileCountX = static_cast<unsigned int>( ( activeViewport.Width + TILE_RES - 1 )
                / static_cast<float>( TILE_RES ) );
            const unsigned int tileCountY = static_cast<unsigned int>( ( activeViewport.Height + TILE_RES - 1 )
                / static_cast<float>( TILE_RES ) );

            // Start GPU Compute
            cmdList->dispatchComputeCmd( tileCountX, tileCountY, 1 );

            // Explicitely unbind the uav buffer
            lightIndexBuffer->unbind( cmdList );
            depthBuffer->unbind( cmdList );
        }
    );

    // If a light culling pass is added, implicitly import the render entity buffer in the current pipeline for the light passes
    entityBuffer.buffer = renderableEntitiesBuffer.get();
    renderPipeline->importWellKnownResource( &entityBuffer );

    return RenderPass.buffers[0];
}