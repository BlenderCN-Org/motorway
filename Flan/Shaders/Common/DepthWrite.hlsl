struct VertexStageData
{
    float3 Position     : POSITION;
    float3 normal       : NORMAL;
    float2 uvCoord      : TEXCOORD0;
};

#include <CameraData.hlsli>
#include <Tessellation.hlsli>

struct PixelDepthShaderData
{
// NOTE Use SV_POSITION semantic after the tesselation step of the pipeline
#if PH_HEIGHTFIELD
    float4 position		: POSITION;
#else
    float4 position		: SV_POSITION;
#endif
    float2 uvCoord      : TEXCOORD0;
    
#if PH_HEIGHTFIELD
    float  tesselationFactor : TESS;
    float2 vertexBounds : POSITION1;
#endif
};

cbuffer MatricesBuffer : register( b3 )
{
    float4x4	ModelMatrix;
    float4x4	g_DepthViewProjectionMatrix;
};

PixelDepthShaderData EntryPointVS( VertexStageData VertexBuffer )
{
    PixelDepthShaderData output = ( PixelDepthShaderData )0;

#if PH_HEIGHTFIELD
    // Send position in model space (projection into depth space should be done at Domain stage)
    output.position = float4( VertexBuffer.Position, 1.0f ); 
    output.tesselationFactor = CalcTessFactor( VertexBuffer.Position );
    output.vertexBounds = VertexBuffer.normal.xy;
#else
    float4 positionWS       = mul( ModelMatrix, float4( VertexBuffer.Position, 1.0f ) );
    
    // NOTE This actually write non-linear depth to the render target/buffer/whatever is bind...
    // You still need to do the division with the projection matrix to get proper values!
    output.position = mul( float4( positionWS.xyz, 1.0f ), g_DepthViewProjectionMatrix );
#endif

    output.uvCoord = VertexBuffer.uvCoord;

    return output;
}
