struct VertexBufferData
{
    float4 Position         : POSITION;
    float2 TexCoordinates   : TEXCOORD0;
    float4 Color            : COLOR0;
};

struct VertexOutput
{
    float4 Position         : SV_POSITION;
    float4 Color            : COLOR0;
    float2 TexCoordinates   : TEXCOORD0;
    float  OutlineThickness : TEXCOORD1;
};

cbuffer ScreenInfos : register( b0 )
{
    uint2  g_ScreenSize;
};
    
float2 projectPoint( float2 vPoint )
{
    return ( vPoint * ( 1.0f / ( g_ScreenSize * 0.5f ) ) + float2( -1.0f, 1.0f ) );
}

VertexOutput EntryPointVS( in VertexBufferData VertexBuffer )
{
    VertexOutput output;

    output.Position = float4( projectPoint( VertexBuffer.Position.xy ), VertexBuffer.Position.z, 1 );
    output.Color = VertexBuffer.Color;
    output.TexCoordinates = VertexBuffer.TexCoordinates;
    output.OutlineThickness = VertexBuffer.Position.w;

    return output;
}

Texture2D g_FontAtlasTexture : register( t0 );
sampler g_LinearSampler : register( s0 );

float contour( in float f, in float d, in float w )
{
    // smoothstep(lower edge0, upper edge1, x)
    return smoothstep( f - w, f + w, d );
}

float samp( in float2 uv, float w )
{
    return contour( 0.5f, g_FontAtlasTexture.SampleLevel( g_LinearSampler, uv, 0 ).a, w );
}

float4 EntryPointPS( in VertexOutput VertexStage ) : SV_TARGET0
{
    float dist = g_FontAtlasTexture.SampleLevel( g_LinearSampler, VertexStage.TexCoordinates, 0 ).a;
    
    float width = fwidth( dist );

    float alpha = contour( 0.5f, dist, width );

    static const float dscale = 0.354f;
    float2 duv = dscale * ( ddx( VertexStage.TexCoordinates ) + ddy( VertexStage.TexCoordinates ) );
    float4 box = float4( VertexStage.TexCoordinates - duv, VertexStage.TexCoordinates + duv );

    float asum = samp( box.xy, width )
        + samp( box.zw, width )
        + samp( box.xw, width )
        + samp( box.zy, width );

    alpha = ( alpha + 0.5f * asum ) / 3.0f;

    float4 color = VertexStage.Color;

    if ( VertexStage.OutlineThickness > 0.0 ) {
        // Convert Thickness to distance and remap its range from 0..1 to 0..0.5
        float outlineDistance = ( 1.0f - VertexStage.OutlineThickness ) * 0.5f;

        color = lerp( float4( 0, 0, 0, 1 ), VertexStage.Color, alpha );

        // Recompute Alpha
        alpha = contour( outlineDistance, dist, width );
    }

    return float4( color.rgb, color.a * alpha );
}
