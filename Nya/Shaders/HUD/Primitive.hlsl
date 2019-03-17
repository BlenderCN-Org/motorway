#include <InstanceData.hlsli>

struct VertexBufferData
{
	float3 Position         : POSITION0;
	float3 Normal           : NORMAL0;
	float2 TexCoordinates   : TEXCOORD0;
};

struct VertexOutput
{
    float4 Position         : SV_POSITION;
	float2 TexCoordinates   : TEXCOORD0;
};

cbuffer ScreenInfos : register( b0 )
{
    float4x4  g_OrthographicMatrix;
};

VertexOutput EntryPointVS( in VertexBufferData VertexBuffer, uint InstanceId : SV_InstanceID )
{
    float4x4 ModelMatrix = GetInstanceModelMatrix( InstanceId );
  
    float4 PositionSS = mul( ModelMatrix, float4( VertexBuffer.Position.xyz, 1 ) );
    
    VertexOutput output;
    output.Position = mul( float4( PositionSS.xyz, 1.0f ), g_OrthographicMatrix );
    output.TexCoordinates = VertexBuffer.TexCoordinates;
    
    return output;
}

float4 EntryPointPS( in VertexOutput VertexStage ) : SV_TARGET0
{
    return float4( 0, 1, 1, 0.50f );
}
