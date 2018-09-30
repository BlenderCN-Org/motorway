struct VertexStageData
{
    float3 position     : POSITION;
    float3 normal       : NORMAL;
    float2 uvCoord      : TEXCOORD0;
};

struct PixelDepthShaderData
{
    float4 position		: SV_POSITION;
    float2 uvCoord      : TEXCOORD0;
};

cbuffer MatricesBuffer : register( b3 )
{
    float4x4	ModelMatrix;
    float4x4	DepthViewProjectionMatrix;
};

PixelDepthShaderData EntryPointVS( VertexStageData VertexBuffer )
{
    PixelDepthShaderData output = ( PixelDepthShaderData )0;

    float4 PositionWS = mul( ModelMatrix, float4( VertexBuffer.position, 1.0f ) );
   
    // NOTE This actually write non-linear depth to the render target/buffer/whatever is bind...
    // You still need to do the division with the projection matrix to get proper values!
    output.position = mul( float4( PositionWS.xyz, 1.0f ), DepthViewProjectionMatrix );
    output.uvCoord = VertexBuffer.uvCoord;

    return output;
}
