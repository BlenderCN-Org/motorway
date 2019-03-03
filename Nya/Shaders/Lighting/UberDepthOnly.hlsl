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

float4x4 GetInstanceModelMatrix( const uint instanceIdx )
{
    uint modelMatrixVectorOffset = g_StartVector + instanceIdx * g_VectorPerInstance + 0;
    
    float4 r0 = g_InstanceVectorBuffer.Load( modelMatrixVectorOffset + 0 );
    float4 r1 = g_InstanceVectorBuffer.Load( modelMatrixVectorOffset + 1 );
    float4 r2 = g_InstanceVectorBuffer.Load( modelMatrixVectorOffset + 2 );
    float4 r3 = g_InstanceVectorBuffer.Load( modelMatrixVectorOffset + 3 );
    
    return float4x4( r0, r1, r2, r3 );
}

float4x4 GetInstanceViewProjectionMatrix( const uint instanceIdx )
{
    uint vpMatrixVectorOffset = g_StartVector + instanceIdx * g_VectorPerInstance + 4;
    
    float4 r0 = g_InstanceVectorBuffer.Load( vpMatrixVectorOffset + 0 );
    float4 r1 = g_InstanceVectorBuffer.Load( vpMatrixVectorOffset + 1 );
    float4 r2 = g_InstanceVectorBuffer.Load( vpMatrixVectorOffset + 2 );
    float4 r3 = g_InstanceVectorBuffer.Load( vpMatrixVectorOffset + 3 );
    
    return float4x4( r0, r1, r2, r3 );
}

VertexStageData EntryPointVS( VertexBufferData VertexBuffer, uint InstanceId : SV_InstanceID )
{
    VertexStageData output = (VertexStageData)0;
    
    float4x4 ModelMatrix = GetInstanceModelMatrix( InstanceId );
    float4x4 ViewProjection = GetInstanceViewProjectionMatrix( InstanceId );
    
    float2 uvCoordinates =  VertexBuffer.TexCoordinates;
    
#if NYA_SCALE_UV_BY_MODEL_SCALE
    float scaleX = length( float3( ModelMatrix._11, ModelMatrix._12, ModelMatrix._13 ) );
    float scaleY = length( float3( ModelMatrix._21, ModelMatrix._22, ModelMatrix._23 ) );

    uvCoordinates *= float2( scaleX, scaleY );
#endif

    float4 positionWS       = mul( ModelMatrix, float4( VertexBuffer.Position, 1.0f ) );
    output.position         = mul( float4( positionWS.xyz, 1.0f ), ViewProjection );
    output.uvCoord          = uvCoordinates;
	return output;
}

// PixelShader
void EntryPointPS( VertexStageData VertexStage, bool isFrontFace : SV_IsFrontFace )
{
    
}
