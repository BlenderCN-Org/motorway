Texture2D g_TexHeightmap : register( t0 );
RWTexture2D<float3> g_TexNormalMap : register( u0 );

static const int ThreadSize = 16;

[numthreads( ThreadSize, ThreadSize, 1 )]
void EntryPointCS( uint2 id : SV_DispatchThreadID )
{	
	// Sobel filtering (9 bilinear fetches)
	uint2 samplePos0 = uint2( id.x, id.y ) + uint2( -1, -1 );
	uint2 samplePos1 = uint2( id.x, id.y ) + uint2( 0, -1 );
	uint2 samplePos2 = uint2( id.x, id.y ) + uint2( 1, -1 );
	
	uint2 samplePos3 = uint2( id.x, id.y ) + uint2( -1, 0 );
	uint2 samplePos5 = uint2( id.x, id.y ) + uint2( 1, 0 );
	
	uint2 samplePos6 = uint2( id.x, id.y ) + uint2( -1, 1 );
	uint2 samplePos7 = uint2( id.x, id.y ) + uint2( 0, 1 );
	uint2 samplePos8 = uint2( id.x, id.y ) + uint2( 1, 1 );
	
	float sample0 = g_TexHeightmap[samplePos0].r;
	float sample1 = g_TexHeightmap[samplePos1].r;
	float sample2 = g_TexHeightmap[samplePos2].r;
	float sample3 = g_TexHeightmap[samplePos3].r;
	
	float sample5 = g_TexHeightmap[samplePos5].r;
	float sample6 = g_TexHeightmap[samplePos6].r;
	float sample7 = g_TexHeightmap[samplePos7].r;
	float sample8 = g_TexHeightmap[samplePos8].r;
	
	float Gx = sample0 - sample2 + 2.0f * sample3 - 2.0f * sample5 + sample6 - sample8;		
	float Gy = sample0 + 2.0f * sample1 + sample2 - sample6 - 2.0f * sample7 - sample8;
	float Gz = 0.5f * sqrt( 1.0f - Gx * Gx - Gy * Gy );

	// Make sure the returned normal is of unit length
	float3 normalWorldSpace = normalize( float3( 2.0f * Gx, 2.0f * Gz, Gy ) );
	
	g_TexNormalMap[id] = normalWorldSpace;
}
