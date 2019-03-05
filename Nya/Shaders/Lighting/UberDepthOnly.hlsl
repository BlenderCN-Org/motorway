#include <CameraData.hlsli>

Buffer<float4> g_InstanceVectorBuffer : register( t8 );

cbuffer InstanceBuffer : register( b1 )
{
    float   g_StartVector;
    float   g_VectorPerInstance;
};

struct VertexBufferData
{
	float3 Position         : POSITION0;
	float3 Normal           : NORMAL0;
	float2 TexCoordinates   : TEXCOORD0;
};

struct VertexStageData
{
    float4 position		: SV_POSITION;
    float2 uvCoord      : TEXCOORD0;
};

float4x4 GetInstanceModelViewProjectionMatrix( const uint instanceIdx )
{
    uint vectorOffset = g_StartVector + instanceIdx * g_VectorPerInstance + 0;
    
    float4 r0 = g_InstanceVectorBuffer.Load( vectorOffset + 0 );
    float4 r1 = g_InstanceVectorBuffer.Load( vectorOffset + 1 );
    float4 r2 = g_InstanceVectorBuffer.Load( vectorOffset + 2 );
    float4 r3 = g_InstanceVectorBuffer.Load( vectorOffset + 3 );
    
    return float4x4( r0, r1, r2, r3 );
}

VertexStageData EntryPointVS( VertexBufferData VertexBuffer, uint InstanceId : SV_InstanceID )
{
    VertexStageData output = (VertexStageData)0;
    
    float4x4 ModelViewProjectionMatrix = GetInstanceModelViewProjectionMatrix( InstanceId );
    
    float2 uvCoordinates =  VertexBuffer.TexCoordinates;
    
    output.position = mul( ModelViewProjectionMatrix, float4( VertexBuffer.Position, 1.0f ) );
    output.uvCoord = uvCoordinates;
	return output;
}

// PixelShader
void EntryPointPS( VertexStageData VertexStage, bool isFrontFace : SV_IsFrontFace )
{
    
}
