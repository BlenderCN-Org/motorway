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
#include "LightGrid.h"

#include <Graphics/ShaderCache.h>
#include <Graphics/GraphicsAssetCache.h>
#include <Graphics/RenderPipeline.h>

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

#include <Maths/Helpers.h>

using namespace nya::maths;

static constexpr int CLUSTER_X = 32;
static constexpr int CLUSTER_Y = 8;
static constexpr int CLUSTER_Z = 32;

LightGrid::LightGrid( BaseAllocator* allocator )
    : memoryAllocator( allocator )
    , lightCullingPso( nullptr )
    , sceneInfosBuffer{ 0 }
    , PointLightCount( 0 )
    , DirectionalLightCount( 0 )
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
            bufferDesc.viewFormat = eImageFormat::IMAGE_FORMAT_R32_UINT;
            bufferDesc.width = CLUSTER_X;
            bufferDesc.height = CLUSTER_Y;
            bufferDesc.depth = CLUSTER_Z;
            bufferDesc.mipCount = 1;

            passData.lightsClusters = renderPipelineBuilder.allocateBuffer( bufferDesc, SHADER_STAGE_COMPUTE );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            Buffer* lightsClusters = renderPipelineResources.getBuffer( passData.lightsClusters );
            
            Buffer* lightsBuffer = renderPipelineResources.getBuffer( passData.lightsBuffer );
            cmdList->updateBuffer( lightsBuffer, &lights, sizeof( lights ) );

            Buffer* lightsClustersInfos = renderPipelineResources.getBuffer( passData.lightsClustersInfosBuffer );
            cmdList->updateBuffer( lightsClustersInfos, &sceneInfosBuffer, sizeof( SceneInfosBuffer ) );

            ResourceListDesc resListDesc = {};
            resListDesc.uavBuffers[0] = { 0, SHADER_STAGE_COMPUTE, lightsClusters };
            resListDesc.constantBuffers[0] = { 2, SHADER_STAGE_COMPUTE, lightsBuffer };
            resListDesc.constantBuffers[1] = { 1, SHADER_STAGE_COMPUTE, lightsClustersInfos };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            cmdList->bindPipelineState( lightCullingPso );
            
            cmdList->dispatchCompute( MAX_POINT_LIGHT_COUNT / 16u, MAX_POINT_LIGHT_COUNT / 16u, 1u );
        }
    );

    return passData;
}

void LightGrid::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache )
{
    PipelineStateDesc pipelineState = {};
    pipelineState.computeShader = shaderCache->getOrUploadStage( "Lighting/LightCulling", eShaderStage::SHADER_STAGE_COMPUTE );

    lightCullingPso = renderDevice->createPipelineState( pipelineState );
}

void LightGrid::setSceneBounds( const nyaVec3f& sceneAABBMax, const nyaVec3f& sceneAABBMin )
{
    sceneInfosBuffer.SceneAABBMax = sceneAABBMax;
    sceneInfosBuffer.SceneAABBMin = sceneAABBMin;

    updateClustersInfos();
}

#define NYA_IMPL_LIGHT_ALLOC( lightType, lightMacroName )\
lightType##Data* LightGrid::allocate##lightType##Data( const lightType##Data&& lightData )\
{\
    if ( lightType##Count >= MAX_##lightMacroName##_COUNT ) {\
        NYA_CERR << "Too many " << #lightType << "! (max is set to " << MAX_##lightMacroName##_COUNT << ")" << std::endl;\
        return nullptr;\
    }\
\
    lightType##Data& light = lights.lightType##s[lightType##Count++];\
    light = std::move( lightData );\
\
    return &light;\
}

NYA_IMPL_LIGHT_ALLOC( DirectionalLight, DIRECTIONAL_LIGHT )
NYA_IMPL_LIGHT_ALLOC( PointLight, POINT_LIGHT )

#undef NYA_IMPL_LIGHT_ALLOC

void LightGrid::updateClustersInfos()
{
    sceneInfosBuffer.ClustersScale = nyaVec3f( float( CLUSTER_X ), float( CLUSTER_Y ), float( CLUSTER_Z ) ) / ( sceneInfosBuffer.SceneAABBMax - sceneInfosBuffer.SceneAABBMin );
    sceneInfosBuffer.ClustersInverseScale = 1.0f / sceneInfosBuffer.ClustersScale;
    sceneInfosBuffer.ClustersBias = -sceneInfosBuffer.ClustersScale * sceneInfosBuffer.SceneAABBMin;
}
