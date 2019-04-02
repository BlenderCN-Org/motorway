#include <PhotometricHelpers.hlsli>

Texture2D g_TextureInput : register( t20 );
sampler g_BilinearSampler : register( s0 );

struct VertexStageData
{
	float4	Position : SV_POSITION;
	float2	TexCoordinates : TEXCOORD;
};

float4 EntryPointPS( in VertexStageData VertexStage ) : SV_Target0
{
    float textureWidth, textureHeight;
    g_TextureInput.GetDimensions( textureWidth, textureHeight );

    float2 textureDimensions = float2( textureWidth, textureHeight );
    float2 inverseTextureDimensions = 1 / textureDimensions;

    float2 TexelUV = VertexStage.TexCoordinates.xy * textureDimensions;
    float2 Texel = floor( TexelUV );
    float2 UV = ( Texel + 0.5 ) * inverseTextureDimensions;

    // Custom hand-crafted 36-texel downsample (13 bilinear fetches)
    float4 Fetch01 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( -1, -1 ) );
    float4 Fetch02 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( +1, -1 ) );
    float4 Fetch03 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( +1, +1 ) );
    float4 Fetch04 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( -1, +1 ) );

    float4 Fetch05 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( -2, -2 ) );
    float4 Fetch06 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( +0, -2 ) );
    float4 Fetch07 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( +0, +0 ) );
    float4 Fetch08 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( -2, +0 ) );

    float4 Fetch09 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( +2, -2 ) );
    float4 Fetch10 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( +2, +0 ) );
    float4 Fetch11 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( +2, +2 ) );
    float4 Fetch12 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( +0, +2 ) );
    float4 Fetch13 = g_TextureInput.SampleLevel( g_BilinearSampler, UV, 0, int2( -2, +2 ) );

#if NYA_USE_KARIS_AVERAGE
    // Partial Karis Average (apply the Karis average in blocks of 4 samples)
    float4 BlockFetch01 = ( Fetch01 + Fetch02 + Fetch03 + Fetch04 );
    float Weight01 = 1.0 / ( RGBToLuminance( BlockFetch01.rgb ) + 1 );

    float4 BlockFetch02 = ( Fetch05 + Fetch06 + Fetch07 + Fetch08 );
    float Weight02 = 1.0 / ( RGBToLuminance( BlockFetch02.rgb ) + 1 );

    float4 BlockFetch03 = ( Fetch06 + Fetch09 + Fetch10 + Fetch07 );
    float Weight03 = 1.0 / ( RGBToLuminance( BlockFetch03.rgb ) + 1 );

    float4 BlockFetch04 = ( Fetch07 + Fetch10 + Fetch11 + Fetch12 );
    float Weight04 = 1.0 / ( RGBToLuminance( BlockFetch04.rgb ) + 1 );

    float4 BlockFetch05 = ( Fetch08 + Fetch07 + Fetch12 + Fetch13 );
    float Weight05 = 1.0 / ( RGBToLuminance( BlockFetch05.rgb ) + 1 );
    
    // Weighting fetches
    float4 Weighted1 = BlockFetch01 * Weight01;
    float4 Weighted2 = BlockFetch02 * Weight02;
    float4 Weighted3 = BlockFetch03 * Weight03;
    float4 Weighted4 = BlockFetch04 * Weight04;
    float4 Weighted5 = BlockFetch05 * Weight05;

    // Compute the weight sum (for normalization)
    float WeightSum = ( Weight01 + Weight02 + Weight03 + Weight04 + Weight05 );
    float InvWeightSum = 1 / WeightSum;
#else
    // Weighting fetches
    float4 Weighted1 = ( Fetch01 + Fetch02 + Fetch03 + Fetch04 ) * 0.500f;
    float4 Weighted2 = ( Fetch05 + Fetch06 + Fetch07 + Fetch08 ) * 0.125f;
    float4 Weighted3 = ( Fetch06 + Fetch09 + Fetch10 + Fetch07 ) * 0.125f;
    float4 Weighted4 = ( Fetch07 + Fetch10 + Fetch11 + Fetch12 ) * 0.125f;
    float4 Weighted5 = ( Fetch08 + Fetch07 + Fetch12 + Fetch13 ) * 0.125f;
    
    static const float InvWeightSum = 1.0f;
#endif

    // Sum them' up
    return ( Weighted1 + Weighted2 + Weighted3 + Weighted4 + Weighted5 ) * InvWeightSum;
}
