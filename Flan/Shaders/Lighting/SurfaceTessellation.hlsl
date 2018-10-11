struct VertexStageData
{
    float4 positionWS   : POSITION0;
    float3 normal       : NORMAL0;
    float2 uvCoord      : TEXCOORD0;
    float3 positionMS   : POSITION1;
};

struct HullStageData
{
    float4 positionWS   : POSITION0;
    float3 normal       : NORMAL0;
    float2 uvCoord      : TEXCOORD0;
    float3 positionMS   : POSITION1;
};

struct DomainStageData
{
    float4 position		: SV_POSITION;
    float4 positionWS   : POSITION0;
    float4 previousPosition : POSITION1;
    float3 normal       : NORMAL0;
    float depth         : DEPTH;
    float2 uvCoord      : TEXCOORD0;
	
#if PH_HEIGHTFIELD
    float3 positionMS   : POSITION2;
#endif
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3]         : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor          : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};
 
#define NUM_CONTROL_POINTS 3
 
// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT ComputeHeightfieldPatchConstants( InputPatch<VertexStageData, NUM_CONTROL_POINTS> ip, uint PatchID : SV_PrimitiveID )
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
HullStageData EntryPointHS( InputPatch<VertexStageData, NUM_CONTROL_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
    HullStageData output;
 
    // Passthrough function
    output.positionWS = ip[i].positionWS;
    output.normal = ip[i].normal;
    output.uvCoord = ip[i].uvCoord;
    output.positionMS = ip[i].positionMS;
 
    return output;
}

#define NUM_CONTROL_POINTS 3
 
#include <CameraData.hlsli>

[domain("tri")]
DomainStageData EntryPointDS(
    HS_CONSTANT_DATA_OUTPUT input,
    float3 domain : SV_DomainLocation,
    const OutputPatch<HullStageData, NUM_CONTROL_POINTS> patch)
{
    DomainStageData output;
 
    output.positionMS = float4(patch[0].positionMS.xyz*domain.x + patch[1].positionMS.xyz*domain.y + patch[2].positionMS.xyz*domain.z, 1.0f);
    
    float4 positionWS = float4( patch[0].positionWS.xyz * domain.x + patch[1].positionWS.xyz * domain.y + patch[2].positionWS.xyz * domain.z, 1.0f );
 
    output.position = mul( float4( positionWS.xyz, 1.0f ), ViewProjectionMatrix );
    output.previousPosition = mul( float4( positionWS.xyz, 1.0f ), g_PreviousViewProjectionMatrix ); //.xywz;
   
    output.positionWS = positionWS;
    output.normal = float4(patch[0].normal.xyz*domain.x + patch[1].normal.xyz*domain.y + patch[2].normal.xyz*domain.z, 1.0f);
    
    float4 PositionVS = mul( float4( positionWS.xyz, 1.0f ), ViewMatrix );
    output.depth = ( PositionVS.z / PositionVS.w );
    
    output.uvCoord = patch[0].uvCoord;
     
    return output;
}
