#include <PhotometricHelpers.hlsli>

Texture2D g_TextureInput : register( t0 );
sampler g_BilinearSampler : register( s0 );

struct VertexStageData
{
	float4	Position : SV_POSITION;
	float2	TexCoordinates : TEXCOORD;
};

float3 EntryPointPS( in VertexStageData VertexStage ) : SV_Target0
{
    float3 color = g_TextureInput.Sample( g_BilinearSampler, VertexStage.TexCoordinates ).rgb;
    
    float luminance = RGBToLuminance( color );
    float intensity = max( luminance - 1.0f, 0.0f );

    return ( color * intensity );
}
