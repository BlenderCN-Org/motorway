/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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
#include "LightGrid.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/GraphicsAssetCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

#include <Maths/Helpers.h>

using namespace nya::maths;

static constexpr int CLUSTER_X = 16;
static constexpr int CLUSTER_Y = 8;
static constexpr int CLUSTER_Z = 24;

LightGrid::LightGrid( BaseAllocator* allocator )
    : memoryAllocator( allocator )
    , lightCullingPso( nullptr )
    , sceneInfosBuffer{ 0 }
    , pointLightCount( 0 )
    , localIBLProbeCount( 0 )
    , lights{}
{

}

LightGrid::~LightGrid()
{

}

void LightGrid::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( lightCullingPso );
}

LightGrid::PassData LightGrid::updateClusters( RenderPipeline* renderPipeline )
{
    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Light Clusters Update Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            BufferDesc lightBufferDesc = {};
            lightBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            lightBufferDesc.size = sizeof( lights );
            
            passData.lightsBuffer = renderPipelineBuilder.allocateBuffer( lightBufferDesc, SHADER_STAGE_COMPUTE );

            BufferDesc sceneClustersBufferDesc = {};
            sceneClustersBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            sceneClustersBufferDesc.size = sizeof( SceneInfosBuffer );

            passData.lightsClustersInfosBuffer = renderPipelineBuilder.allocateBuffer( sceneClustersBufferDesc, SHADER_STAGE_COMPUTE );

            BufferDesc bufferDesc = {};
            bufferDesc.type = BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D;
            bufferDesc.viewFormat = eImageFormat::IMAGE_FORMAT_R32G32_UINT;
            bufferDesc.width = CLUSTER_X;
            bufferDesc.height = CLUSTER_Y;
            bufferDesc.depth = CLUSTER_Z;
            bufferDesc.mipCount = 1u;

            passData.lightsClusters = renderPipelineBuilder.allocateBuffer( bufferDesc, SHADER_STAGE_COMPUTE );

            bufferDesc = {};
            bufferDesc.type = BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER;
            bufferDesc.viewFormat = eImageFormat::IMAGE_FORMAT_R32_UINT;
            bufferDesc.size = sizeof( uint32_t ) * CLUSTER_X * CLUSTER_Y * CLUSTER_Z * ( MAX_POINT_LIGHT_COUNT + MAX_LOCAL_IBL_PROBE_COUNT );
            bufferDesc.stride = static_cast<uint32_t>( bufferDesc.size / sizeof( uint32_t ) );
            
            passData.itemList = renderPipelineBuilder.allocateBuffer( bufferDesc, SHADER_STAGE_COMPUTE );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice ) {
            Buffer* lightsClusters = renderPipelineResources.getBuffer( passData.lightsClusters );
            Buffer* itemList = renderPipelineResources.getBuffer( passData.itemList );
            Buffer* lightsBuffer = renderPipelineResources.getBuffer( passData.lightsBuffer );
            Buffer* lightsClustersInfos = renderPipelineResources.getBuffer( passData.lightsClustersInfosBuffer );

            ResourceList resourceList;
            resourceList.resource[0].buffer = lightsClusters;
            resourceList.resource[1].buffer = itemList;
            resourceList.resource[2].buffer = lightsBuffer;
            resourceList.resource[3].buffer = lightsClustersInfos;
            renderDevice->updateResourceList( lightCullingPso, resourceList );

            CommandList& cmdList = renderDevice->allocateComputeCommandList();
            {
                cmdList.begin();

                cmdList.updateBuffer( lightsBuffer, &lights, sizeof( lights ) );
                cmdList.updateBuffer( lightsClustersInfos, &sceneInfosBuffer, sizeof( SceneInfosBuffer ) );

                cmdList.bindPipelineState( lightCullingPso );

                cmdList.dispatchCompute( 16u, 8u, 24u );

                cmdList.end();
            }

            renderDevice->submitCommandList( &cmdList );
        }
    );

    return passData;
}

void LightGrid::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache )
{
    PipelineStateDesc pipelineState = {};
    pipelineState.computeShader = shaderCache->getOrUploadStage( "Lighting/LightCulling", eShaderStage::SHADER_STAGE_COMPUTE );
    pipelineState.resourceListLayout.resources[0] = { 0, SHADER_STAGE_COMPUTE, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_UAV_TEXTURE };
    pipelineState.resourceListLayout.resources[1] = { 1, SHADER_STAGE_COMPUTE, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_UAV_BUFFER };
    pipelineState.resourceListLayout.resources[2] = { 2, SHADER_STAGE_COMPUTE, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
    pipelineState.resourceListLayout.resources[3] = { 4, SHADER_STAGE_COMPUTE, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };

    lightCullingPso = renderDevice->createPipelineState( pipelineState );
}

void LightGrid::setSceneBounds( const nyaVec3f& sceneAABBMax, const nyaVec3f& sceneAABBMin )
{
    sceneInfosBuffer.SceneAABBMax = sceneAABBMax;
    sceneInfosBuffer.SceneAABBMin = sceneAABBMin;

    updateClustersInfos();
}

PointLightData* LightGrid::allocatePointLightData( const PointLightData&& lightData )
{
    if ( localIBLProbeCount >= MAX_POINT_LIGHT_COUNT ) {
        NYA_CERR << "Too many Point Lights! (max is set to " << MAX_POINT_LIGHT_COUNT << ")" << std::endl;
        return nullptr;
    }

    PointLightData& light = lights.PointLights[pointLightCount++]; \
    light = std::move( lightData );

    return &light;
}

IBLProbeData* LightGrid::allocateLocalIBLProbeData( const IBLProbeData&& probeData )
{
    if ( localIBLProbeCount >= MAX_LOCAL_IBL_PROBE_COUNT ) {
        NYA_CERR << "Too many Local IBL Probes! (max is set to " << MAX_LOCAL_IBL_PROBE_COUNT << ")" << std::endl;
        return nullptr;
    }

    // NOTE Offset probe array index (first probe should be the global IBL probe)
    const uint16_t probeIndex = ( 1u + localIBLProbeCount++ );

    IBLProbeData& light = lights.IBLProbes[probeIndex];
    light = std::move( probeData );
    light.ProbeIndex = probeIndex;

    return &light;
}

DirectionalLightData* LightGrid::updateDirectionalLightData( const DirectionalLightData&& lightData )
{
    lights.DirectionalLight = std::move( lightData );
    return &lights.DirectionalLight;
}

IBLProbeData* LightGrid::updateGlobalIBLProbeData( const IBLProbeData&& probeData )
{
    lights.IBLProbes[0] = std::move( probeData );
    lights.IBLProbes[0].ProbeIndex = 0u;

    return &lights.IBLProbes[0];
}

const DirectionalLightData* LightGrid::getDirectionalLightData() const
{
    return &lights.DirectionalLight;
}

const IBLProbeData* LightGrid::getGlobalIBLProbeData() const
{
    return &lights.IBLProbes[0];
}

void LightGrid::updateClustersInfos()
{
    sceneInfosBuffer.ClustersScale = nyaVec3f( float( CLUSTER_X ), float( CLUSTER_Y ), float( CLUSTER_Z ) ) / ( sceneInfosBuffer.SceneAABBMax - sceneInfosBuffer.SceneAABBMin );
    sceneInfosBuffer.ClustersInverseScale = 1.0f / sceneInfosBuffer.ClustersScale;
    sceneInfosBuffer.ClustersBias = -sceneInfosBuffer.ClustersScale * sceneInfosBuffer.SceneAABBMin;
}
