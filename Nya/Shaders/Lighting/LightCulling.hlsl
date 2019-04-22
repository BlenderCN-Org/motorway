#include <LightsData.hlsli>
#include <SceneInfos.hlsli>

static const int NUM_THREADS_X = 1;
static const int NUM_THREADS_Y = 1;
static const int NUM_THREADS_Z = 1;
static const int NUM_THREADS = ( NUM_THREADS_X * NUM_THREADS_Y * NUM_THREADS_Z );

static const int CLUSTER_X = 16;
static const int CLUSTER_Y = 8;
static const int CLUSTER_Z = 24;

static const uint CLUSTER_COUNT = ( CLUSTER_Z * CLUSTER_Y * CLUSTER_X );

RWTexture3D<uint2> g_LightClusters : register( u0 );
RWBuffer<uint> g_ItemList : register( u1 );

groupshared uint g_ClusterItems[( MAX_POINT_LIGHT_COUNT + MAX_LOCAL_IBL_PROBE_COUNT )];

[numthreads( NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z )]
void EntryPointCS( uint3 globalIdx : SV_DispatchThreadID, uint3 localIdx : SV_GroupThreadID, uint3 groupIdx : SV_GroupID, uint groupIndex : SV_GroupIndex )
{	
	int3 groupClusterOffset = int3( groupIdx * int3( NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z ) );
	int3 clusterIdx = int3( localIdx ) + groupClusterOffset;
    // uint localIdxFlattened = localIdx.x + localIdx.y * NUM_THREADS_X + localIdx.z * NUM_THREADS_X * NUM_THREADS_Y;
	// uint localItemListOffset = localIdxFlattened * ( MAX_POINT_LIGHT_COUNT + MAX_LOCAL_IBL_PROBE_COUNT );
	
    uint threadListOffset = 0u;
    
	// PointLight Culling
    for ( uint i = 0; i < MAX_POINT_LIGHT_COUNT; i++ ) {
		PointLight light = g_PointLights[i];
		
        const float3 p = ( light.PositionAndRadius.xyz - g_SceneAABBMin );
        const float3 pMin = ( p - light.PositionAndRadius.w ) * g_ClustersScale;
        const float3 pMax = ( p + light.PositionAndRadius.w ) * g_ClustersScale;

        // Cluster for the center of the light
        const int px = ( int )floor( p.x * g_ClustersScale.x );
        const int py = ( int )floor( p.y * g_ClustersScale.y );
        const int pz = ( int )floor( p.z * g_ClustersScale.z );
	
        // Cluster bounds for the light
        const int x0 = max( ( int )floor( pMin.x ), 0 );
        const int x1 = min( ( int )ceil( pMax.x ), CLUSTER_X );
        const int y0 = max( ( int )floor( pMin.y ), 0 );
        const int y1 = min( ( int )ceil( pMax.y ), CLUSTER_Y );
        const int z0 = max( ( int )floor( pMin.z ), 0 );
        const int z1 = min( ( int )ceil( pMax.z ), CLUSTER_Z );
        
		const float squaredRadius = light.PositionAndRadius.w * light.PositionAndRadius.w;
		
		// Skip if light is outside current cluster bounds
		[branch]
		if ( clusterIdx.x < x0 
			|| clusterIdx.x > x1 
			|| clusterIdx.y < y0 
			|| clusterIdx.y > y1 
			|| clusterIdx.z < z0 
			|| clusterIdx.z > z1 ) {
			continue;
		}
		
		float dz = ( pz == clusterIdx.z ) 
			? 0.0f 
			: g_SceneAABBMin.z + ( ( pz < clusterIdx.z ) ? clusterIdx.z : clusterIdx.z + 1 ) * g_ClustersInverseScale.z - light.PositionAndRadius.z;
		dz *= dz;
		
		float dy = ( py == clusterIdx.y ) 
			? 0.0f 
			: g_SceneAABBMin.y + ( ( py < clusterIdx.y ) ? clusterIdx.y : clusterIdx.y + 1 ) * g_ClustersInverseScale.y - light.PositionAndRadius.y;
		dy *= dy;
		dy += dz;
		
		float dx = ( px == clusterIdx.x ) 
			? 0.0f 
			: g_SceneAABBMin.x + ( ( px < clusterIdx.x ) ? clusterIdx.x : clusterIdx.x + 1 ) * g_ClustersInverseScale.x - light.PositionAndRadius.x;
		dx *= dx;
		dx += dy;
	            			
		if ( dx < squaredRadius ) {
			g_ClusterItems[threadListOffset] = i;
            threadListOffset++;
		}
	}
    
    const uint pointLightCount = threadListOffset;
	
	// IBLProbe Culling
    for ( uint j = 0; j < MAX_LOCAL_IBL_PROBE_COUNT; j++ ) {
		IBLProbe probe = g_IBLProbes[j + 1u];
		
        const float3 p = ( probe.PositionAndRadius.xyz - g_SceneAABBMin );
        const float3 pMin = ( p - probe.PositionAndRadius.w ) * g_ClustersScale;
        const float3 pMax = ( p + probe.PositionAndRadius.w ) * g_ClustersScale;

        // Cluster for the center of the light
        const int px = ( int )floor( p.x * g_ClustersScale.x );
        const int py = ( int )floor( p.y * g_ClustersScale.y );
        const int pz = ( int )floor( p.z * g_ClustersScale.z );
	
        // Cluster bounds for the light
        const int x0 = max( ( int )floor( pMin.x ), 0 );
        const int x1 = min( ( int )ceil( pMax.x ), CLUSTER_X );
        const int y0 = max( ( int )floor( pMin.y ), 0 );
        const int y1 = min( ( int )ceil( pMax.y ), CLUSTER_Y );
        const int z0 = max( ( int )floor( pMin.z ), 0 );
        const int z1 = min( ( int )ceil( pMax.z ), CLUSTER_Z );
        
		const float squaredRadius = probe.PositionAndRadius.w * probe.PositionAndRadius.w;
		
		// Skip if probe is outside current cluster bounds
		[branch]
		if ( clusterIdx.x < x0 
			|| clusterIdx.x > x1 
			|| clusterIdx.y < y0 
			|| clusterIdx.y > y1 
			|| clusterIdx.z < z0 
			|| clusterIdx.z > z1 ) {
			continue;
		}
		
		float dz = ( pz == clusterIdx.z ) 
			? 0.0f 
			: g_SceneAABBMin.z + ( ( pz < clusterIdx.z ) ? clusterIdx.z : clusterIdx.z + 1 ) * g_ClustersInverseScale.z - probe.PositionAndRadius.z;
		dz *= dz;
		
		float dy = ( py == clusterIdx.y ) 
			? 0.0f 
			: g_SceneAABBMin.y + ( ( py < clusterIdx.y ) ? clusterIdx.y : clusterIdx.y + 1 ) * g_ClustersInverseScale.y - probe.PositionAndRadius.y;
		dy *= dy;
		dy += dz;
		
		float dx = ( px == clusterIdx.x ) 
			? 0.0f 
			: g_SceneAABBMin.x + ( ( px < clusterIdx.x ) ? clusterIdx.x : clusterIdx.x + 1 ) * g_ClustersInverseScale.x - probe.PositionAndRadius.x;
		dx *= dx;
		dx += dy;
	            			
		if ( dx < squaredRadius ) {
			g_ClusterItems[threadListOffset] = j;
            threadListOffset++;
		}
	}
    
    uint iblProbeCount = ( threadListOffset - pointLightCount );	
    uint spotLightCount = 0u;
	
	uint itemListOffset = ( globalIdx.x + globalIdx.y * CLUSTER_X + globalIdx.z * CLUSTER_X * CLUSTER_Y ) * ( MAX_POINT_LIGHT_COUNT + MAX_LOCAL_IBL_PROBE_COUNT );	
    g_LightClusters[globalIdx] = uint2( itemListOffset, ( pointLightCount << 20 ) & 0xFFF00000 | ( spotLightCount << 8 ) & 0x000FFF00 | ( iblProbeCount & 0x000000FF ) );
	
    // Write from LDS memory to UAV
	for ( int x = 0; x < ( MAX_POINT_LIGHT_COUNT + MAX_LOCAL_IBL_PROBE_COUNT ); x++ )
		g_ItemList[itemListOffset+x] = g_ClusterItems[x];
}
