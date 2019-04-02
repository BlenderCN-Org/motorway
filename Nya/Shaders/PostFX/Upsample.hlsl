Texture2D g_TextureInput : register( t20 );
sampler g_BilinearSampler : register( s0 );

struct VertexStageData
{
	float4	Position : SV_POSITION;
	float2	TexCoordinates : TEXCOORD;
};

cbuffer UpsampleInfos : register( b0 )
{
    float2  g_InverseTextureDimensions; // = 1.0f / Input texture 
    float   g_FilterRadius; // NOTE The lower the resolution is, the bigger the radius should be
}

float4 EntryPointPS( in VertexStageData VertexStage ) : SV_Target0
{
    float4 d = float4( g_InverseTextureDimensions, g_InverseTextureDimensions ) * float4( 1, 1, -1, 0 ) * g_FilterRadius;

    // 3x3 Tent Filter
    //             -------  
    //            | 1 2 1 |
    // ( 1 / 16 ) | 2 4 2 |
    //            | 1 2 1 |
    //             -------
    float4 Fetch01 = g_TextureInput.Sample( g_BilinearSampler, VertexStage.TexCoordinates - d.xy );
    float4 Fetch02 = g_TextureInput.Sample( g_BilinearSampler, VertexStage.TexCoordinates - d.wy ) * 2;
    float4 Fetch03 = g_TextureInput.Sample( g_BilinearSampler, VertexStage.TexCoordinates - d.zy );

    float4 Fetch04 = g_TextureInput.Sample( g_BilinearSampler, VertexStage.TexCoordinates + d.zw ) * 2;
    float4 Fetch05 = g_TextureInput.Sample( g_BilinearSampler, VertexStage.TexCoordinates ) * 4;
    float4 Fetch06 = g_TextureInput.Sample( g_BilinearSampler, VertexStage.TexCoordinates + d.xw ) * 2;

    float4 Fetch07 = g_TextureInput.Sample( g_BilinearSampler, VertexStage.TexCoordinates + d.zy );
    float4 Fetch08 = g_TextureInput.Sample( g_BilinearSampler, VertexStage.TexCoordinates + d.wy ) * 2;
    float4 Fetch09 = g_TextureInput.Sample( g_BilinearSampler, VertexStage.TexCoordinates + d.xy );
    
    // Sample the previous mip (since we want to accumulate mip levels)
    float4 CurrentMip = ( Fetch01 + Fetch02 + Fetch03 + Fetch04 + Fetch05 + Fetch06 + Fetch07 + Fetch08 + Fetch09 ) * ( 1.0 / 16 );

    // Clamp upsampled result into safe ranges for R11G11B10 storage
    return min( float4( 65519, 65519, 65519, 65519 ), CurrentMip );
}
