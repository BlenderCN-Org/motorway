#include <CameraData.hlsli>
#include <InstanceData.hlsli>

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

VertexStageData EntryPointVS( VertexBufferData VertexBuffer, uint InstanceId : SV_InstanceID )
{
    VertexStageData output = (VertexStageData)0;
    
    float4x4 ModelViewProjectionMatrix = GetInstanceModelMatrix( InstanceId );
    
    float2 uvCoordinates =  VertexBuffer.TexCoordinates;
    
    output.position = mul( ModelViewProjectionMatrix, float4( VertexBuffer.Position, 1.0f ) );
    output.uvCoord = uvCoordinates;
	return output;
}

// PixelShader
void EntryPointPS( VertexStageData VertexStage, bool isFrontFace : SV_IsFrontFace )
{
    
}
