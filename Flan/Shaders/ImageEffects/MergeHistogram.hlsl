// From https://github.com/Patapom/GodComplex

#include <Colormetry.hlsli>
#include <SharedAutoExposure.hlsli>

cbuffer PassData : register( b2 )
{
   uint2 BackbufferDimension;
}

#define NUMTHREADX	1
#define NUMTHREADY	512	// NOTE: This exceeds the advised amount of threads but I don't want to split this shader into multiple passes again
#define NUMTHREADZ	1	// 512 vertical threads ensures a maximum vertical image size of 2048 so we're pretty safe on that side for now...

Buffer<uint>	histogramBins : register( t1 );	// The histogram from last pass used as a RO texture!
RWBuffer<uint>	finalHistogram : register( u0 );	// The final histogram

groupshared uint	Rows[NUMTHREADY];	// Here we're going to read many rows from the multi-rows histogram

[numthreads( NUMTHREADX, NUMTHREADY, NUMTHREADZ )]
void EntryPointCS( uint3 GroupID : SV_GROUPID, uint3 GroupThreadID : SV_GROUPTHREADID, uint3 DispatchThreadID : SV_DISPATCHTHREADID )
{
    uint2	Dimensions = uint2( ( ( BackbufferDimension.y + 3 ) >> 2 ), 128 );

    // Initialize source histogram rows
    uint	RowIndex = NUMTHREADY * GroupID.y + GroupThreadID.y;
    Rows[GroupThreadID.y] = RowIndex < Dimensions.y ? histogramBins[GroupID.x + ( RowIndex * HISTOGRAM_BUCKETS_COUNT )] : 0;
    GroupMemoryBarrierWithGroupSync();

    // Add the 64 last rows to their corresponding 64 first rows
    if ( GroupThreadID.y < 64 ) {
        Rows[GroupThreadID.y] += Rows[GroupThreadID.y + 64];
    }
    GroupMemoryBarrierWithGroupSync();

    // Add the 32 last rows to their corresponding 32 first rows
    if ( GroupThreadID.y < 32 ) {
        Rows[GroupThreadID.y] += Rows[GroupThreadID.y + 32];
    }
    GroupMemoryBarrierWithGroupSync();

    // Add the 16 last rows to their corresponding 16 first rows
    if ( GroupThreadID.y < 16 ) {
        Rows[GroupThreadID.y] += Rows[GroupThreadID.y + 16];
    }
    GroupMemoryBarrierWithGroupSync();

    // Add the 8 last rows to their corresponding 8 first rows
    if ( GroupThreadID.y < 8 ) {
        Rows[GroupThreadID.y] += Rows[GroupThreadID.y + 8];
    }
    GroupMemoryBarrierWithGroupSync();

    // Add the 4 last rows to their corresponding 4 first rows
    if ( GroupThreadID.y < 4 ) {
        Rows[GroupThreadID.y] += Rows[GroupThreadID.y + 4];
    }
    GroupMemoryBarrierWithGroupSync();

    if ( GroupThreadID.y == 0 ) {
        finalHistogram[GroupID.x] = Rows[0] + Rows[1] + Rows[2] + Rows[3];
    }
}
