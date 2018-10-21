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

struct VertexStageHeightfieldData
{
    float4 positionMS   : POSITION; // NOTE Declared as 4D vector to allow compatibility with depthwrite shader
    float2 uvCoord      : TEXCOORD;
    float4 tileInfos    : POSITION1; // xy tile height bounds z skirt id w tileInfos.w
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
    float InsideTessFactor[2]       : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
    uint Skirt : SKIRT;
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

// returns true if the box is completely behind the plane.
bool aabbBehindPlaneTest(float3 center, float3 extents, float4 plane) {
	float3 n = abs(plane.xyz); // |n.x|, |n.y|, |n.z|

	float e = dot(extents, n); // always positive

	float s = dot(float4(center, 1.0f), plane); // signed distance from center point to plane

	// if the center point of the box is a distance of e or more behind the plane
	// (in which case s is negative since it is behind the plane), then the box
	// is completely in the negative half space of the plane.
	return (s + e) < 0.0f;
}

// returns true if the box is completely outside the frustum.
bool aabbOutsideFrustumTest(float3 center, float3 extents, float4 frustumPlanes[6]) {
	for (int i = 0; i < 6; ++i) {
		// if the box is completely behind any of the frustum planes, then it is outside the frustum.
		if (aabbBehindPlaneTest(center, extents, frustumPlanes[i])) {
			return true;
		}
	}

	return false;
}

#include <Tessellation.hlsli>

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT ConstantHS( InputPatch<VertexStageHeightfieldData, NUM_CONTROL_POINTS> ip, uint PatchID : SV_PrimitiveID )
{
    HS_CONSTANT_DATA_OUTPUT output;

	output.Skirt = (uint)ip[0].tileInfos.z;
   
#ifdef PH_DEPTH_WRITE
    output.EdgeTessFactor[0] = 32.0f;
    output.EdgeTessFactor[1] = 32.0f;
    output.EdgeTessFactor[2] = 32.0f;
    output.EdgeTessFactor[3] =32.0f;
    output.InsideTessFactor[0] = 32.0f;
    output.InsideTessFactor[1] = output.InsideTessFactor[0];
    
    return output;
#else
	float3 vMin = float3(ip[0].positionMS.x, ip[0].tileInfos.x * g_Layers[0].HeightmapWorldHeight - 0.5f, ip[0].positionMS.z);
	float3 vMax = float3(ip[3].positionMS.x, ip[0].tileInfos.y * g_Layers[0].HeightmapWorldHeight + 0.5f, ip[3].positionMS.z);
	
	// center/extents representation.
	float3 boxCenter = 0.5f * (vMin + vMax);
	float3 boxExtents = 0.5f * (vMax - vMin);

	if ( aabbOutsideFrustumTest( boxCenter, boxExtents, g_CameraFrustumPlanes ) ) {
		output.EdgeTessFactor[0] = 0.0f;
		output.EdgeTessFactor[1] = 0.0f;
		output.EdgeTessFactor[2] = 0.0f;
		output.EdgeTessFactor[3] = 0.0f;
		output.InsideTessFactor[0] = 0.0f;
		output.InsideTessFactor[1] = 0.0f;

		return output;
	}
#endif

    float3 e0 = 0.5f * (ip[0].positionMS.xyz + ip[2].positionMS.xyz);
    float3 e1 = 0.5f * (ip[0].positionMS.xyz + ip[1].positionMS.xyz);
    float3 e2 = 0.5f * (ip[1].positionMS.xyz + ip[3].positionMS.xyz);
    float3 e3 = 0.5f * (ip[2].positionMS.xyz + ip[3].positionMS.xyz);
    float3 c = 0.25f * (ip[0].positionMS.xyz + ip[1].positionMS.xyz + ip[2].positionMS.xyz + ip[3].positionMS.xyz);

    output.EdgeTessFactor[0] = CalcTessFactor(e0);
    output.EdgeTessFactor[1] = CalcTessFactor(e1);
    output.EdgeTessFactor[2] = CalcTessFactor(e2);
    output.EdgeTessFactor[3] = CalcTessFactor(e3);
    output.InsideTessFactor[0] = CalcTessFactor(c);
    output.InsideTessFactor[1] = output.InsideTessFactor[0];

    return output;
}

[domain("quad")]
[partitioning("fractional_even")]
// #ifdef PH_DEPTH_WRITE
// [outputtopology("triangle_cw")]
// #else
[outputtopology("triangle_ccw")]
//#endif
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullStageData EntryPointHS( InputPatch<VertexStageHeightfieldData, NUM_CONTROL_POINTS> ip, 
                            uint i : SV_OutputControlPointID, 
                            uint PatchID : SV_PrimitiveID )
{
    HullStageData output;
 
    // Passthrough function
    output.positionMS = ip[i].positionMS.xyz;
    output.uvCoord = ip[i].uvCoord;
 
    return output;
}

Texture2D g_TexHeightmap    : register( t0 );
Texture2D g_TexHeightmapNormal : register( t1 );
Texture2D<uint> g_TexHeightmapDisplacement : register( t2 );
Texture2D<uint4> g_TexSplatMap : register( t17 );
struct TerrainSamplingParameter
{
    float4 samplingParameters;
    uint splatIndex;
    uint3 EXPLICIT_PADDING;
};

cbuffer Terrain : register( b7 )
{
	TerrainSamplingParameter g_TerrainMaterials[256];
};

Texture2DArray 		g_TerrainBaseColorHeightArray : register( t6 );

sampler g_HeightmapSampler : register( s8 );
sampler g_DisplacementSampler : register( s9 );

float3 estimateNormal(float2 texcoord) {
    static const float hmapSize = 512.0f;
    static const float bumpHeightScale = 8.0f;
    
    float me = g_TexHeightmap.SampleLevel( g_HeightmapSampler, texcoord, 0 ).x;
    float n = g_TexHeightmap.SampleLevel( g_HeightmapSampler,float2(texcoord.x,texcoord.y+1.0/hmapSize), 0 ).x;
    float s = g_TexHeightmap.SampleLevel( g_HeightmapSampler,float2(texcoord.x,texcoord.y-1.0/hmapSize), 0 ).x;
    float e = g_TexHeightmap.SampleLevel( g_HeightmapSampler,float2(texcoord.x+1.0/hmapSize,texcoord.y), 0 ).x;
    float w = g_TexHeightmap.SampleLevel( g_HeightmapSampler,float2(texcoord.x-1.0/hmapSize,texcoord.y), 0 ).x;     
    
    //find perpendicular vector to norm:
    float3 norm = float3( 0, 1, 0 );
    
    float3 temp = norm; //a temporary vector that is not parallel to norm
    if(norm.x==1)
        temp.y+=0.5;
    else
        temp.x+=0.5;
    //form a basis with norm being one of the axes:
    float3 perp1 = normalize(cross(norm,temp));
    float3 perp2 = normalize(cross(norm,perp1));
    //use the basis to move the normal in its own space by the offset        
    float3 normalOffset = -bumpHeightScale*(((n-me)-(s-me))*perp1 + ((e-me)-(w-me))*perp2);
    norm += normalOffset;
    return normalize(norm);

	// float2 leftTex = texcoord + float2(-1.0f / (float)512, 0.0f);
	// float2 rightTex = texcoord + float2(1.0f / (float)512, 0.0f);
	// float2 bottomTex = texcoord + float2(0.0f, 1.0f / (float)512);
	// float2 topTex = texcoord + float2(0.0f, -1.0f / (float)512);

	// float leftZ = g_TexHeightmap[leftTex].r * g_Layers[0].HeightmapWorldHeight;
	// float rightZ = g_TexHeightmap[rightTex].r * g_Layers[0].HeightmapWorldHeight;
	// float bottomZ = g_TexHeightmap[bottomTex].r * g_Layers[0].HeightmapWorldHeight;
	// float topZ = g_TexHeightmap[topTex].r * g_Layers[0].HeightmapWorldHeight;

	// return normalize(float3(leftZ - rightZ, 8.0f, topZ - bottomZ));
}

[domain("quad")]
DomainStageData EntryPointDS(
    HS_CONSTANT_DATA_OUTPUT input,
    float2 domain : SV_DomainLocation,
    const OutputPatch<HullStageData, NUM_CONTROL_POINTS> patch )
{
    DomainStageData output;
 
	float3 positionMS = lerp(
            lerp( patch[0].positionMS, patch[1].positionMS, domain.x ), 
            lerp( patch[2].positionMS, patch[3].positionMS, domain.x ), 
            domain.y );
    
    output.uvCoord =  lerp(lerp(patch[0].uvCoord, patch[1].uvCoord, domain.x), lerp(patch[2].uvCoord, patch[3].uvCoord, domain.x), domain.y);
    
    positionMS.y = g_TexHeightmap.SampleLevel( g_HeightmapSampler, output.uvCoord, 0.0f ).r * g_Layers[0].HeightmapWorldHeight;
   
	float4 positionWS = mul( ModelMatrix, float4( positionMS.xyz, 1.0f ) );
   
    float3 norm = estimateNormal( output.uvCoord ); 
    
    uint4 splatmapData = g_TexSplatMap.Load( int3( positionMS.xz, 0 ) );
    
	// Retrieve texture coordinates
	float4 samplingParameters = g_TerrainMaterials[splatmapData.r / 257].samplingParameters; // xy offset; zw scale
	float2 samplingCoordinates = (  output.uvCoord.xy + samplingParameters.xy ) * samplingParameters.zw;
    float2 samplingCoordinatesFar = samplingCoordinates * 0.1f;
        
	uint streamedTextureIndex = g_TerrainMaterials[splatmapData.r / 257].splatIndex;
    
    float disp = g_TerrainBaseColorHeightArray.SampleLevel( g_DisplacementSampler, float3( samplingCoordinates.xy, 0 ), streamedTextureIndex ).a * 0.3;
    disp = disp * 2.0f - 1.0f;
          
    positionWS.xyz += ( norm * disp );
    
#if PH_DEPTH_ONLY
    output.position = mul( float4( positionWS.xyz, 1.0f ), g_DepthViewProjectionMatrix );
#else
    output.positionWS = positionWS;
	output.positionMS = positionMS;
	
    output.position = mul( float4( positionWS.xyz, 1.0f ), ViewProjectionMatrix );
    output.previousPosition = mul( float4( positionWS.xyz, 1.0f ), g_PreviousViewProjectionMatrix ); //.xywz;
   
    output.normal = norm;
    
    float4 PositionVS = mul( float4( positionWS.xyz, 1.0f ), ViewMatrix );
    output.depth = ( PositionVS.z / PositionVS.w );
#endif
    
    return output;
}
