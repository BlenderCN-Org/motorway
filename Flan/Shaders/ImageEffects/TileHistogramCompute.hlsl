// From https://github.com/Patapom/GodComplex

#include <Colormetry.hlsli>
#include "SharedAutoExposure.hlsli"

#define NUMTHREADX	32	// Tiles of 32x4 pixels
#define NUMTHREADY	4
#define NUMTHREADZ	1

cbuffer PassData : register( b2 )
{
   uint2 BackbufferDimension;
}

Texture2D<float4>	sourceTextureSRV : register( t0 );			// Source image
RWBuffer<uint>	histogramBuffer : register( u0 );			// The histogram used as a RW texture

static const uint	PASSES_COUNT = HISTOGRAM_BUCKETS_COUNT / NUMTHREADX;	// To address the entire histogram, we need to make each thread in the group process several buckets
                                                                            // A typical histogram size of 128 and 32 threads per group will require 4 passes to address all 4*32=128 buckets...

groupshared uint	Histogram[NUMTHREADY][HISTOGRAM_BUCKETS_COUNT];

[numthreads( NUMTHREADX, NUMTHREADY, NUMTHREADZ )]
void EntryPointCS( uint3 GroupID : SV_GROUPID, uint3 GroupThreadID : SV_GROUPTHREADID, uint3 DispatchThreadID : SV_DISPATCHTHREADID )
{
    // Clear the histogram
    [unroll]
    for ( uint i = 0; i < PASSES_COUNT; i++ ) {
        uint BucketIndex = NUMTHREADX * i + GroupThreadID.x;
        Histogram[GroupThreadID.y][BucketIndex] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    // Accumulate an entire scanline of the source image
    // NUMTHREADY scanlines of the source image are processed at the same time
    uint2 PixelIndex;
    PixelIndex.y = NUMTHREADY * GroupID.y + GroupThreadID.y;	// The source image's scanline we're reading from

    for ( uint X = 0; X < BackbufferDimension.x; X += NUMTHREADX ) {				// Jump in blocks of NUMTHREADX which is our granularity of horizontal blocks (so a 1920 image will require each thread to read 1920/32=60 pixels)
        PixelIndex.x = X + GroupThreadID.x;
        if ( PixelIndex.x >= BackbufferDimension.x || PixelIndex.y >= BackbufferDimension.y )
            continue;	// Don't accumulate pixels outside the screen otherwise we have a bias toward blacks!

        float Luminance = dot( LUMINANCE, sourceTextureSRV[PixelIndex].xyz );
        Luminance *= BISOU_TO_WORLD_LUMINANCE;	// BISOU TO WORLD (bisou units => cd/m²)

        // Reject black pixels that are clearly an error!
        if ( Luminance < MIN_ADAPTABLE_SCENE_LUMINANCE )
            continue;

        float Luminance_BucketIndex = Luminance2HistogramBucketIndex( Luminance );	// 20 * log10( Luminance ) => Histo bucket

        uint BucketIndex = uint( floor( Luminance_BucketIndex ) );
        BucketIndex = clamp( BucketIndex, 0, HISTOGRAM_BUCKETS_COUNT - 1 );		// Writing outside the target's range [0,127] crashes the driver!

        uint old;
        InterlockedAdd( Histogram[GroupThreadID.y][BucketIndex], 1, old );
    }
    GroupMemoryBarrierWithGroupSync();

    // Collapse second half of 2 lines into first half
    // (only working if threads count is at least 4)
    if ( GroupThreadID.y < 2 ) {
        [unroll]
        for ( uint i = 0; i < PASSES_COUNT; i++ ) {
            uint BucketIndex = NUMTHREADX * i + GroupThreadID.x;
            Histogram[GroupThreadID.y][BucketIndex] += Histogram[2 + GroupThreadID.y][BucketIndex];
        }
    }
    GroupMemoryBarrierWithGroupSync();

    // From there, we only need a single line of threads to finalize the histogram
    if ( GroupThreadID.y == 0 ) {
        [unroll]
        for ( uint i = 0; i < PASSES_COUNT; i++ ) {
            uint BucketIndex = NUMTHREADX * i + GroupThreadID.x;
            histogramBuffer[BucketIndex + ( GroupID.y * HISTOGRAM_BUCKETS_COUNT )] = Histogram[0][BucketIndex] + Histogram[1][BucketIndex];	// Add the last two lines together and store into destination
        }
    }
}
