struct VertexStageData
{
    float3 Position     : POSITION;
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

#if PH_HEIGHTFIELD
Texture2D g_TexHeightmap    : register( t0 );

#include <MaterialsShared.h>
cbuffer MaterialEdition : register( b8 )
{
    // Flags
    uint                    g_WriteVelocity; // Range: 0..1 (should be 0 for transparent surfaces)
    uint                    g_EnableAlphaTest;
    uint                    g_EnableAlphaBlend;
    uint                    g_IsDoubleFace;
    
    uint                    g_CastShadow;
    uint                    g_ReceiveShadow;
    uint                    g_EnableAlphaToCoverage;
    uint                    g_LayerCount;
    
    MaterialLayer           g_Layers[MAX_LAYER_COUNT];
};
#endif

PixelDepthShaderData EntryPointVS( VertexStageData VertexBuffer )
{
    PixelDepthShaderData output = ( PixelDepthShaderData )0;

#if PH_HEIGHTFIELD
    float2 heightCoords = float2( VertexBuffer.Position.x, VertexBuffer.Position.z );
    
	float3 positionModelSpace = VertexBuffer.Position;
    float height = g_TexHeightmap[heightCoords].r;

    float4 positionWS       = mul( ModelMatrix, float4( positionModelSpace.x, height * g_Layers[0].HeightmapWorldHeight, positionModelSpace.z, 1.0f ) );
#else
    float4 positionWS       = mul( ModelMatrix, float4( VertexBuffer.Position, 1.0f ) );
#endif

    // NOTE This actually write non-linear depth to the render target/buffer/whatever is bind...
    // You still need to do the division with the projection matrix to get proper values!
    output.position = mul( float4( positionWS.xyz, 1.0f ), DepthViewProjectionMatrix );
    output.uvCoord = VertexBuffer.uvCoord;

    return output;
}
