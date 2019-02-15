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

#include <Maths/Helpers.h>

using namespace nya::maths;

static constexpr int CLUSTER_X = 32;
static constexpr int CLUSTER_Y = 8;
static constexpr int CLUSTER_Z = 32;

LightGrid::LightGrid( BaseAllocator* allocator )
    : memoryAllocator( allocator )
    , lightsBuffer( nullptr )
    , clustersTexture( nullptr )
    , aabbMin( 0, 0, 0 )
    , aabbMax( 0, 0, 0 )
    , PointLightCount( 0 )
    , DirectionalLightCount( 0 )
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
    renderDevice->setDebugMarker( clustersTexture, "Light Clusters" );

    BufferDesc lightBufferDesc = {};
    lightBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
    lightBufferDesc.size = sizeof( lights );

    lightsBuffer = renderDevice->createBuffer( lightBufferDesc );
}

void LightGrid::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyTexture( clustersTexture );
    renderDevice->destroyBuffer( lightsBuffer );
}

void LightGrid::updateClusters( CommandList* cmdList )
{
    NYA_PROFILE( __FUNCTION__ )

    uint32_t lightClusters[CLUSTER_Z][CLUSTER_Y][CLUSTER_X] = {};

    nyaVec3f clusterInvScale = ( 1.0f / clustersInfos.Scale );

    for ( uint32_t pointLightIdx = 0; pointLightIdx < PointLightCount; pointLightIdx++ ) {
        const auto& light = lights.PointLights[pointLightIdx];

        const nyaVec3f p = ( light.worldPosition - aabbMin );
        const nyaVec3f pMin = ( p - light.radius ) * clustersInfos.Scale;
        const nyaVec3f pMax = ( p + light.radius ) * clustersInfos.Scale;

        // Cluster for the center of the light
        const int px = ( int )floorf( p.x * clustersInfos.Scale.x );
        const int py = ( int )floorf( p.y * clustersInfos.Scale.y );
        const int pz = ( int )floorf( p.z * clustersInfos.Scale.z );

        // Cluster bounds for the light
        const int x0 = max( ( int )floorf( pMin.x ), 0 );
        const int x1 = min( ( int )ceilf( pMax.x ), CLUSTER_X );
        const int y0 = max( ( int )floorf( pMin.y ), 0 );
        const int y1 = min( ( int )ceilf( pMax.y ), CLUSTER_Y );
        const int z0 = max( ( int )floorf( pMin.z ), 0 );
        const int z1 = min( ( int )ceilf( pMax.z ), CLUSTER_Z );

        const float squaredRadius = light.radius * light.radius;
        const uint32_t mask = ( 1 << pointLightIdx );

        for ( int z = z0; z < z1; z++ ) {
            float dz = ( pz == z ) ? 0.0f : aabbMin.z + ( ( pz < z ) ? z : z + 1 ) * clusterInvScale.z - light.worldPosition.z;
            dz *= dz;

            for ( int y = y0; y < y1; y++ ) {
                float dy = ( py == y ) ? 0.0f : aabbMin.y + ( ( py < y ) ? y : y + 1 ) * clusterInvScale.y - light.worldPosition.y;
                dy *= dy;
                dy += dz;

                for ( int x = x0; x < x1; x++ ) {
                    float dx = ( px == x ) ? 0.0f : aabbMin.x + ( ( px < x ) ? x : x + 1 ) * clusterInvScale.x - light.worldPosition.x;
                    dx *= dx;
                    dx += dy;

                    if ( dx < squaredRadius ) {
                        lightClusters[z][y][x] |= mask;
                    }
                }
            }
        }
    }

    cmdList->updateTexture3D( clustersTexture, lightClusters, sizeof( uint32_t ), CLUSTER_X, CLUSTER_Y, CLUSTER_Z );
    cmdList->updateBuffer( lightsBuffer, &lights, sizeof( lights ) );
}

void LightGrid::setSceneBounds( const nyaVec3f& sceneAABBMax, const nyaVec3f& sceneAABBMin )
{
    aabbMax = sceneAABBMax;
    aabbMin = sceneAABBMin;

    updateClustersInfos();
}

Buffer* LightGrid::getLightsBuffer() const
{
    return lightsBuffer;
}

Texture* LightGrid::getLightsClusters() const
{
    return clustersTexture;
}

const LightGrid::ClustersInfos&  LightGrid::getClustersInfos() const
{
    return clustersInfos;
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
    clustersInfos.Scale = nyaVec3f( float( CLUSTER_X ), float( CLUSTER_Y ), float( CLUSTER_Z ) ) / ( aabbMax - aabbMin );
    clustersInfos.Bias = -clustersInfos.Scale * aabbMin;
}
