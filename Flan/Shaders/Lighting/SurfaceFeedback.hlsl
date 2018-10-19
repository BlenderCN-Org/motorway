cbuffer PerPass : register( b2 )
{
   float2 g_VirtualTexturePageCount;
   float2 g_VirtualTextureSize;
   float g_MipScaleFactor;
   float g_VirtualTextureIndex;
};

struct VertexStageData
{
    float4 position		: SV_POSITION;
    float2 uvCoord      : TEXCOORD0;
};

float ComputeMipLevel( in float2 uv )
{
    float2 coord_pixels = uv * g_VirtualTextureSize;

    float2 x_deriv = ddx( coord_pixels );
    float2 y_deriv = ddy( coord_pixels );

    float d = max( dot( x_deriv, x_deriv ), dot( y_deriv, y_deriv ) );
    float m = max( ( 0.5 * log2( d ) ) - g_MipScaleFactor, 0.0 );

    return floor( min( m, 11 ) );
}

float4 EntryPointPS( VertexStageData VertexStage ) : SV_TARGET0
{
    float mipLevel = ComputeMipLevel( VertexStage.uvCoord );
    float2 pageCoordinates = floor( VertexStage.uvCoord * g_VirtualTexturePageCount / exp2( mipLevel ) );

    return float4( pageCoordinates, mipLevel, g_VirtualTextureIndex ) / 255.0f;
}
