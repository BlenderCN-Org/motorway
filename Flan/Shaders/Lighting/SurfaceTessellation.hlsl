struct VertexStageHeightfieldData
{
    float3 positionMS   : POSITION;
    float2 uvCoord      : TEXCOORD;
};

struct HullStageData
{
    float3 positionMS   : POSITION;
    float2 uvCoord      : TEXCOORD;
};

struct DomainStageData
{
    float4 position		: SV_POSITION;
    float4 positionWS   : POSITION0;
    float4 previousPosition : POSITION1;
    float3 normal       : NORMAL0;
    float depth         : DEPTH;
    float2 uvCoord      : TEXCOORD0;
    float3 positionMS   : POSITION2;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3]         : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor          : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};
 
#define NUM_CONTROL_POINTS 3
 
// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT ComputeHeightfieldPatchConstants( InputPatch<VertexStageHeightfieldData, NUM_CONTROL_POINTS> ip, uint PatchID : SV_PrimitiveID )
{
    HS_CONSTANT_DATA_OUTPUT output;
 
    // Insert code to compute output here
    output.EdgeTessFactor[0] = 4;
    output.EdgeTessFactor[1] = 4;
    output.EdgeTessFactor[2] = 4;
    output.InsideTessFactor = 4; 
 
    return output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ComputeHeightfieldPatchConstants")]
HullStageData EntryPointHS( InputPatch<VertexStageHeightfieldData, NUM_CONTROL_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
    HullStageData output;
 
    // Passthrough function
    output.positionMS = ip[i].positionMS;
    output.uvCoord = ip[i].uvCoord;
 
    return output;
}

#define NUM_CONTROL_POINTS 3
 
#include <CameraData.hlsli>

Texture2D g_TexHeightmap    : register( t0 );
Texture2D g_TexHeightmapNormal : register( t1 );

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

[domain("tri")]
DomainStageData EntryPointDS(
    HS_CONSTANT_DATA_OUTPUT input,
    float3 domain : SV_DomainLocation,
    const OutputPatch<HullStageData, NUM_CONTROL_POINTS> patch)
{
    DomainStageData output;
 
	float3 positionMS = patch[0].positionMS.xyz*domain.x + patch[1].positionMS.xyz*domain.y + patch[2].positionMS.xyz*domain.z;
    const float2 sampleCoordinates = float2( positionMS.x, positionMS.z );
    float height = g_TexHeightmap[sampleCoordinates].r;
	float4 positionWS = float4( positionMS.x, height * g_Layers[0].HeightmapWorldHeight, positionMS.z, 1.0f );
	
    output.positionWS = positionWS;
	output.positionMS = positionMS;
	
	//mul( ModelMatrix, float4( output.positionMS.x, height * g_Layers[0].HeightmapWorldHeight, output.positionMS.z, 1.0f ) );
	//output.normal = normalize( mul( ModelMatrix, float4( g_TexHeightmapNormal[sampleCoordinates].xyz, 0.0f ) ) ).xyz;
    
    output.position = mul( float4( positionWS.xyz, 1.0f ), ViewProjectionMatrix );
    output.previousPosition = mul( float4( positionWS.xyz, 1.0f ), g_PreviousViewProjectionMatrix ); //.xywz;
   
    output.normal = g_TexHeightmapNormal[sampleCoordinates].xyz;
    
    float4 PositionVS = mul( float4( positionWS.xyz, 1.0f ), ViewMatrix );
    output.depth = ( PositionVS.z / PositionVS.w );
    
    output.uvCoord = patch[0].uvCoord;
     
    return output;
}
