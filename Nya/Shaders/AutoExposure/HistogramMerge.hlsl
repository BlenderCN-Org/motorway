#include <PhotometricHelpers.hlsli>
#include "SharedAutoExposure.hlsli"

#define NUMTHREADX	1
#define NUMTHREADY	512
#define NUMTHREADZ	1

cbuffer PassData : register( b0 )
{
   uint2 BackbufferDimension;
}

Buffer<uint>	histogramBins : register( t0 );
RWBuffer<uint>	finalHistogram : register( u0 );

groupshared uint	Rows[NUMTHREADY];

[numthreads( NUMTHREADX, NUMTHREADY, NUMTHREADZ )]
void EntryPointCS( uint3 GroupID : SV_GROUPID, uint3 GroupThreadID : SV_GROUPTHREADID, uint3 DispatchThreadID : SV_DISPATCHTHREADID )
{
    uint2	Dimensions = uint2( ( ( BackbufferDimension.y + 3 ) >> 2 ), 128 );

    uint	RowIndex = NUMTHREADY * GroupID.y + GroupThreadID.y;
    Rows[GroupThreadID.y] = RowIndex < Dimensions.y ? histogramBins[GroupID.x + ( RowIndex * HISTOGRAM_BUCKETS_COUNT )] : 0;
    GroupMemoryBarrierWithGroupSync();

    if ( GroupThreadID.y < 64 ) {
        Rows[GroupThreadID.y] += Rows[GroupThreadID.y + 64];
    }
    GroupMemoryBarrierWithGroupSync();

    if ( GroupThreadID.y < 32 ) {
        Rows[GroupThreadID.y] += Rows[GroupThreadID.y + 32];
    }
    GroupMemoryBarrierWithGroupSync();

    if ( GroupThreadID.y < 16 ) {
        Rows[GroupThreadID.y] += Rows[GroupThreadID.y + 16];
    }
    GroupMemoryBarrierWithGroupSync();

    if ( GroupThreadID.y < 8 ) {
        Rows[GroupThreadID.y] += Rows[GroupThreadID.y + 8];
    }
    GroupMemoryBarrierWithGroupSync();

    if ( GroupThreadID.y < 4 ) {
        Rows[GroupThreadID.y] += Rows[GroupThreadID.y + 4];
    }
    GroupMemoryBarrierWithGroupSync();

    if ( GroupThreadID.y == 0 ) {
        finalHistogram[GroupID.x] = Rows[0] + Rows[1] + Rows[2] + Rows[3];
    }
}
