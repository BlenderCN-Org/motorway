#include <LightsData.hlsli>
#include <SceneInfos.hlsli>

static const int NUM_THREADS_X = 16;
static const int NUM_THREADS_Y = 1;
static const int NUM_THREADS = ( NUM_THREADS_X * NUM_THREADS_Y );

static const int CLUSTER_X = 32;
static const int CLUSTER_Y = 8;
static const int CLUSTER_Z = 32;

static const uint CLUSTER_COUNT = ( CLUSTER_Z * CLUSTER_Y * CLUSTER_X );

RWTexture3D<uint2> g_LightClusters : register( u0 );

groupshared uint g_LightMasksPointLights[CLUSTER_Z][CLUSTER_Y][CLUSTER_X];

[numthreads( NUM_THREADS_X, NUM_THREADS_Y, 1 )]
void EntryPointCS( uint3 globalIdx : SV_DispatchThreadID, uint3 localIdx : SV_GroupThreadID, uint3 groupIdx : SV_GroupID )
{
    uint localIdxFlattened = localIdx.x + localIdx.y * NUM_THREADS_X;
	
	// Initialize LDS memory
    if ( localIdxFlattened == 0 ) {
		for ( int z = 0; z < CLUSTER_Z; ++z ) {
			for ( int y = 0; y < CLUSTER_Y; ++y ) {
				for ( int x = 0; x < CLUSTER_X; ++x ) {
					g_LightMasksPointLights[z][y][x] = 0u;
				}
			}
		}
	}
	
	[unroll]
    for ( uint i = localIdxFlattened; i < MAX_POINT_LIGHT_COUNT; i += NUM_THREADS ) {
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
        const uint mask = ( 1 << i );
		
		[loop]
        for ( int z = z0; z < z1; z++ ) {
            float dz = ( pz == z ) ? 0.0f : g_SceneAABBMin.z + ( ( pz < z ) ? z : z + 1 ) * g_ClustersInverseScale.z - light.PositionAndRadius.z;
            dz *= dz;
            
			[loop]
            for ( int y = y0; y < y1; y++ ) {
                float dy = ( py == y ) ? 0.0f : g_SceneAABBMin.y + ( ( py < y ) ? y : y + 1 ) * g_ClustersInverseScale.y - light.PositionAndRadius.y;
                dy *= dy;
                dy += dz;
				
				[loop]
                for ( int x = x0; x < x1; x++ ) {
                    float dx = ( px == x ) ? 0.0f : g_SceneAABBMin.x + ( ( px < x ) ? x : x + 1 ) * g_ClustersInverseScale.x - light.PositionAndRadius.x;
                    dx *= dx;
                    dx += dy;
                     
					[branch]             			
                    if ( dx < squaredRadius ) {
                        g_LightMasksPointLights[z][y][x] |= mask;
                    }
                }
            }
        }
	}
	
    GroupMemoryBarrierWithGroupSync();
    
    if ( localIdxFlattened == 0 ) {
        // Copy Point Light Masks
		for ( int z = 0; z < CLUSTER_Z; ++z ) {
			for ( int y = 0; y < CLUSTER_Y; ++y ) {
				for ( int x = 0; x < CLUSTER_X; ++x ) {
					g_LightClusters[uint3( x, y, z )] = uint2( g_LightMasksPointLights[z][y][x], 0u );
				}
			}
		}
    }
        
    if ( localIdxFlattened == 0 ) {
		for ( int z = 0; z < CLUSTER_Z; ++z ) {
			for ( int y = 0; y < CLUSTER_Y; ++y ) {
				for ( int x = 0; x < CLUSTER_X; ++x ) {
					g_LightMasksPointLights[z][y][x] = 0u;
				}
			}
		}
    }
    
    [unroll]
    for ( uint j = localIdxFlattened; j < MAX_LOCAL_IBL_PROBE_COUNT; j += NUM_THREADS ) {
		IBLProbe probe = g_IBLProbes[j + 1u];
		
        const float3 p = ( probe.PositionAndRadius.xyz - g_SceneAABBMin );
        const float3 pMin = ( p - probe.PositionAndRadius.w ) * g_ClustersScale;
        const float3 pMax = ( p + probe.PositionAndRadius.w ) * g_ClustersScale;

        // Cluster for the center of the probe
        const int px = ( int )floor( p.x * g_ClustersScale.x );
        const int py = ( int )floor( p.y * g_ClustersScale.y );
        const int pz = ( int )floor( p.z * g_ClustersScale.z );

        // Cluster bounds for the probe
        const int x0 = max( ( int )floor( pMin.x ), 0 );
        const int x1 = min( ( int )ceil( pMax.x ), CLUSTER_X );
        const int y0 = max( ( int )floor( pMin.y ), 0 );
        const int y1 = min( ( int )ceil( pMax.y ), CLUSTER_Y );
        const int z0 = max( ( int )floor( pMin.z ), 0 );
        const int z1 = min( ( int )ceil( pMax.z ), CLUSTER_Z );

        const float squaredRadius = probe.PositionAndRadius.w * probe.PositionAndRadius.w;
        const uint mask = ( 1 << j );
		
		[loop]
        for ( int z = z0; z < z1; z++ ) {
            float dz = ( pz == z ) ? 0.0f : g_SceneAABBMin.z + ( ( pz < z ) ? z : z + 1 ) * g_ClustersInverseScale.z - probe.PositionAndRadius.z;
            dz *= dz;
            
			[loop]
            for ( int y = y0; y < y1; y++ ) {
                float dy = ( py == y ) ? 0.0f : g_SceneAABBMin.y + ( ( py < y ) ? y : y + 1 ) * g_ClustersInverseScale.y - probe.PositionAndRadius.y;
                dy *= dy;
                dy += dz;
				
				[loop]
                for ( int x = x0; x < x1; x++ ) {
                    float dx = ( px == x ) ? 0.0f : g_SceneAABBMin.x + ( ( px < x ) ? x : x + 1 ) * g_ClustersInverseScale.x - probe.PositionAndRadius.x;
                    dx *= dx;
                    dx += dy;
                     
					[branch]             			
                    if ( dx < squaredRadius ) {
                        g_LightMasksPointLights[z][y][x] |= mask;
                    }
                }
            }
        }
	}
	
    GroupMemoryBarrierWithGroupSync();
	
    if ( localIdxFlattened == 0 ) {
        // Copy IBL Probe Masks
		for ( int z = 0; z < CLUSTER_Z; ++z ) {
			for ( int y = 0; y < CLUSTER_Y; ++y ) {
				for ( int x = 0; x < CLUSTER_X; ++x ) {
					g_LightClusters[uint3( x, y, z )] = uint2( g_LightClusters[uint3( x, y, z )].r, g_LightMasksPointLights[z][y][x] );
				}
			}
		}
	}
}
