#include <PhotometricHelpers.hlsli>
#include "SharedAutoExposure.hlsli"

#define NUMTHREADX	32
#define NUMTHREADY	4
#define NUMTHREADZ	1

cbuffer PassData : register( b0 )
{
   uint2 BackbufferDimension;
}

Texture2D<float4> sourceTextureSRV : register( t0 );
RWBuffer<uint> histogramBuffer : register( u0 );

static const uint	PASSES_COUNT = HISTOGRAM_BUCKETS_COUNT / NUMTHREADX;
groupshared uint	Histogram[NUMTHREADY][HISTOGRAM_BUCKETS_COUNT];

[numthreads( NUMTHREADX, NUMTHREADY, NUMTHREADZ )]
void EntryPointCS( uint3 GroupID : SV_GROUPID, uint3 GroupThreadID : SV_GROUPTHREADID, uint3 DispatchThreadID : SV_DISPATCHTHREADID )
{
    [unroll]
    for ( uint i = 0; i < PASSES_COUNT; i++ ) {
        uint BucketIndex = NUMTHREADX * i + GroupThreadID.x;
        Histogram[GroupThreadID.y][BucketIndex] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    uint2 PixelIndex;
    PixelIndex.y = NUMTHREADY * GroupID.y + GroupThreadID.y;

    for ( uint X = 0; X < BackbufferDimension.x; X += NUMTHREADX ) {
        PixelIndex.x = X + GroupThreadID.x;
        if ( PixelIndex.x >= BackbufferDimension.x || PixelIndex.y >= BackbufferDimension.y )
            continue;
        
        float Luminance = RGBToLuminance( sourceTextureSRV[PixelIndex].xyz ) * BISOU_TO_WORLD_LUMINANCE;
        
        if ( Luminance < MIN_ADAPTABLE_SCENE_LUMINANCE )
            continue;

        float Luminance_BucketIndex = Luminance2HistogramBucketIndex( Luminance );

        uint BucketIndex = uint( floor( Luminance_BucketIndex ) );
        BucketIndex = clamp( BucketIndex, 0, HISTOGRAM_BUCKETS_COUNT - 1 );

        uint old;
        InterlockedAdd( Histogram[GroupThreadID.y][BucketIndex], 1, old );
    }
    GroupMemoryBarrierWithGroupSync();

    if ( GroupThreadID.y < 2 ) {
        [unroll]
        for ( uint i = 0; i < PASSES_COUNT; i++ ) {
            uint BucketIndex = NUMTHREADX * i + GroupThreadID.x;
            Histogram[GroupThreadID.y][BucketIndex] += Histogram[2 + GroupThreadID.y][BucketIndex];
        }
    }
    GroupMemoryBarrierWithGroupSync();

    if ( GroupThreadID.y == 0 ) {
        [unroll]
        for ( uint i = 0; i < PASSES_COUNT; i++ ) {
            uint BucketIndex = NUMTHREADX * i + GroupThreadID.x;
            histogramBuffer[BucketIndex + ( GroupID.y * HISTOGRAM_BUCKETS_COUNT )] = Histogram[0][BucketIndex] + Histogram[1][BucketIndex];
        }
    }
}