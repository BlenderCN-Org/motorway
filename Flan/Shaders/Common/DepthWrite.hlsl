struct VertexStageData
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
    float2 uvCoord      : TEXCOORD0;
};

#include <CameraData.hlsli>
#include <Tessellation.hlsli>

#if PH_HEIGHTFIELD
Texture2D g_TexHeightmap    : register( t0 );
sampler g_HeightmapSampler : register( s8 );
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
    float4 tileInfos    : POSITION1; // xy tile height bounds z skirt id w tesselationFactor
#endif
};

cbuffer MatricesBuffer : register( b3 )
{
    float4x4	ModelMatrix;
    float4x4	g_DepthViewProjectionMatrix;
    uint        g_EnableAlphaStippling;
};

PixelDepthShaderData EntryPointVS( VertexStageData VertexBuffer )
{
    PixelDepthShaderData output = ( PixelDepthShaderData )0;

#if PH_HEIGHTFIELD
    const float2 sampleCoordinates = float2( VertexBuffer.Position.x, VertexBuffer.Position.z );
    
    // Send position in model space (projection into depth space should be done at Domain stage)
    output.position = float4( VertexBuffer.Position, 1.0f ); 
    
    output.position.y = g_TexHeightmap.SampleLevel( g_HeightmapSampler, VertexBuffer.uvCoord, 0.0f ).r * g_Layers[0].HeightmapWorldHeight;
    output.tileInfos = float4( VertexBuffer.Normal, 0.0f );
#else
    float4 positionWS       = mul( ModelMatrix, float4( VertexBuffer.Position, 1.0f ) );
    
    // NOTE This actually write non-linear depth to the render target/buffer/whatever is bind...
    // You still need to do the division with the projection matrix to get proper values!
    output.position = mul( float4( positionWS.xyz, 1.0f ), g_DepthViewProjectionMatrix );
#endif

    output.uvCoord = VertexBuffer.uvCoord;

    return output;
}
