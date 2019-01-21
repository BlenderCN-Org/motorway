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

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

#include <glm/glm/glm.hpp>

static constexpr unsigned int CLUSTER_X = 32;
static constexpr unsigned int CLUSTER_Y = 8;
static constexpr unsigned int CLUSTER_Z = 32;

LightGrid::LightGrid( BaseAllocator* allocator )
    : memoryAllocator( allocator )
    , lightsBuffer( nullptr )
    , clustersTexture( nullptr )
    , aabbMin( 0, 0, 0 )
    , aabbMax( 0, 0, 0 )
    , lights{}
{

}

LightGrid::~LightGrid()
{

}

void LightGrid::create( RenderDevice* renderDevice )
{
    TextureDescription clustersDesc = {};
    clustersDesc.dimension = TextureDescription::DIMENSION_TEXTURE_3D;
    clustersDesc.format = eImageFormat::IMAGE_FORMAT_R32_UINT;
    clustersDesc.width = CLUSTER_X;
    clustersDesc.height = CLUSTER_Y;
    clustersDesc.depth = CLUSTER_Z;
    clustersDesc.arraySize = 1;
    clustersDesc.mipCount = 1;
    clustersDesc.samplerCount = 1;
    clustersDesc.flags.allowCPUWrite = 1;

    clustersTexture = renderDevice->createTexture3D( clustersDesc );
}

void LightGrid::updateClusters( CommandList* cmdList )
{
    uint32_t lights[CLUSTER_Z][CLUSTER_Y][CLUSTER_X] = {};

    glm::vec3 worldSize = ( aabbMax - aabbMin );
    glm::vec3 clusterInvScale = 1.0f / glm::vec3( static_cast< float >( CLUSTER_X ), static_cast< float >( CLUSTER_Y ), static_cast< float >( CLUSTER_Z ) ) / worldSize;

    cmdList->updateTexture( clustersTexture, lights, sizeof( lights ) );
}

void LightGrid::setSceneBounds( const glm::vec3& sceneAABBMax, const glm::vec3& sceneAABBMin )
{
    aabbMax = sceneAABBMax;
    aabbMin = sceneAABBMin;
}
//
//DirectionalLight& LightGrid::allocateDirectionalLight( const DirectionalLightData&& lightData )
//{
//
//}
//
//PointLight&         allocatePointLight( const PointLightData&& lightData );
//SpotLight&          allocateSpotLight( const SpotLightData&& lightData );
//
//EnvironmentProbe&   allocateLocalEnvironmentProbe( const EnvironmentProbeData&& lightData );
//EnvironmentProbe&   allocateGlobalEnvironmentProbe( const EnvironmentProbeData&& lightData );
