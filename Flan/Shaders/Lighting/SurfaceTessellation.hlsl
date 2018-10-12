struct VertexStageHeightfieldData
{
    float4 positionMS   : POSITION; // NOTE Declared as 4D vector to allow compatibility with depthwrite shader
    float2 uvCoord      : TEXCOORD;
};

struct HullStageData
{
    float3 positionMS   : POSITION;
    float2 uvCoord      : TEXCOORD;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[4]         : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor[2]          : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};
 
#include <CameraData.hlsli>

#if PH_DEPTH_ONLY
cbuffer MatricesBuffer : register( b3 )
{
    float4x4	ModelMatrix;
    float4x4	g_DepthViewProjectionMatrix;
};

struct DomainStageData
{
    float4 position		: SV_POSITION;
    float2 uvCoord      : TEXCOORD0;
};
#else
cbuffer MatricesBuffer : register( b3 )
{
    float4x4	ModelMatrix;
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
#endif

#define NUM_CONTROL_POINTS 4

float CalcTessFactor(float3 p) 
{
    float d = distance(p, WorldPosition);

    float s = saturate((d - 16.0f) / (512.0f - 16.0f));
    return pow(2, (lerp(6, 0, s)));
}

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT ComputeHeightfieldPatchConstants( InputPatch<VertexStageHeightfieldData, NUM_CONTROL_POINTS> ip, uint PatchID : SV_PrimitiveID )
{
    HS_CONSTANT_DATA_OUTPUT output;
     float tessellationAmount;

        float3 distance = ip[PatchID].positionMS.xyz - WorldPosition.xyz;

        if (length(distance) > 2.0f)
            tessellationAmount = 4.0f;
        else
            tessellationAmount = 64.0f / length(distance);

    float3 e0 = 0.5f * (ip[0].positionMS.xyz + ip[2].positionMS.xyz);
    float3 e1 = 0.5f * (ip[0].positionMS.xyz + ip[1].positionMS.xyz);
    float3 e2 = 0.5f * (ip[1].positionMS.xyz + ip[3].positionMS.xyz);
    float3 e3 = 0.5f * (ip[2].positionMS.xyz + ip[3].positionMS.xyz);
    float3 c = 0.25f * (ip[0].positionMS.xyz + ip[1].positionMS.xyz + ip[2].positionMS.xyz + ip[3].positionMS.xyz);

    output.EdgeTessFactor[0] = 4; //tessellationAmount; //CalcTessFactor(e0);
    output.EdgeTessFactor[1] = 4; //tessellationAmount; //CalcTessFactor(e1);
    output.EdgeTessFactor[2] = 4; //tessellationAmount; //CalcTessFactor(e2);
    output.EdgeTessFactor[3] = 4; //tessellationAmount; //CalcTessFactor(e3);
    output.InsideTessFactor[0] = 4; //CalcTessFactor(c);
    output.InsideTessFactor[1] = output.InsideTessFactor[0];

    return output;
}

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ComputeHeightfieldPatchConstants")]
HullStageData EntryPointHS( InputPatch<VertexStageHeightfieldData, NUM_CONTROL_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
    HullStageData output;
 
    // Passthrough function
    output.positionMS = ip[i].positionMS.xyz;
    output.uvCoord = ip[i].uvCoord;
 
    return output;
}

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

[domain("quad")]
DomainStageData EntryPointDS(
    HS_CONSTANT_DATA_OUTPUT input,
    float2 domain : SV_DomainLocation,
    const OutputPatch<HullStageData, NUM_CONTROL_POINTS> patch)
{
    DomainStageData output;
 
	float3 positionMS = lerp(lerp(patch[0].positionMS, patch[1].positionMS, domain.x), lerp(patch[2].positionMS, patch[3].positionMS, domain.x), domain.y);
    
    const float2 sampleCoordinates = float2( positionMS.x, positionMS.z );
    float height = g_TexHeightmap[sampleCoordinates].r;
	float4 positionWS = float4( positionMS.x, height * g_Layers[0].HeightmapWorldHeight, positionMS.z, 1.0f );
	
#if PH_DEPTH_ONLY
    output.position = mul( float4( positionWS.xyz, 1.0f ), g_DepthViewProjectionMatrix );
#else
    output.positionWS = positionWS;
	output.positionMS = positionMS;
	
    output.position = mul( float4( positionWS.xyz, 1.0f ), ViewProjectionMatrix );
    output.previousPosition = mul( float4( positionWS.xyz, 1.0f ), g_PreviousViewProjectionMatrix ); //.xywz;
   
    output.normal = normalize( mul( ModelMatrix, float4( g_TexHeightmapNormal[sampleCoordinates].xyz, 0.0f ) ) ).xyz;
    
    float4 PositionVS = mul( float4( positionWS.xyz, 1.0f ), ViewMatrix );
    output.depth = ( PositionVS.z / PositionVS.w );
#endif
    
    output.uvCoord =  lerp(lerp(patch[0].uvCoord, patch[1].uvCoord, domain.x), lerp(patch[2].uvCoord, patch[3].uvCoord, domain.x), domain.y);
     
    return output;
}
