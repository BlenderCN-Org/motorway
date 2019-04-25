#include <InstanceData.hlsli>
#include <PhotometricHelpers.hlsli>

struct VertexBufferData
{
	float3 Position         : POSITION0;
	float3 Normal           : NORMAL0;
	float2 TexCoordinates   : TEXCOORD0;
};

struct VertexStageData
{
    float4 position  : SV_POSITION;
	float2 uvCoord   : TEXCOORD0;
};

cbuffer ScreenInfos : register( b0 )
{
    float4x4  g_OrthographicMatrix;
};

VertexStageData EntryPointVS( in VertexBufferData VertexBuffer, uint InstanceId : SV_InstanceID )
{
    float4x4 ModelMatrix = GetInstanceModelMatrix( InstanceId );
  
    float4 PositionSS = mul( ModelMatrix, float4( VertexBuffer.Position.xyz, 1 ) );
    
    VertexStageData output;
    output.position = mul( float4( PositionSS.xyz, 1.0f ), g_OrthographicMatrix );
    output.uvCoord = VertexBuffer.TexCoordinates;
    
    return output;
}

struct MaterialReadLayer
{
    float3  BaseColor;
    float   Reflectance;
    
    float   Roughness;
    float   Metalness;
    float   AmbientOcclusion;
    float   Emissivity;
    
    float3  Normal;
    float   AlphaMask;
    
    float3  SecondaryNormal;
    float   BlendMask;
    
    float   Refraction;
    float   RefractionIor;
    float   ClearCoat;
    float   ClearCoatGlossiness;
    
    float   DiffuseContribution;
    float   SpecularContribution;
    float   NormalContribution;
    float   AlphaCutoff;
    float   SSStrength;
};

sampler					g_BilinearSampler : register( s0 );

#include <MaterialShared.h>

// t0 => Light clusters
// t1 => Sun CSM ShadowMap
// t2..4 => IBL Probes Array (Diffuse/Specular/Captured (HDR))
// t5 => Shading Model BRDF DFG LUT

Texture2D g_TexBaseColor0 : register( t1 );
Texture2D g_TexAlphaMask0 : register( t2 );
Texture2D g_TexReflectance0 : register( t3 );

Texture2D g_TexRoughness0 : register( t4 );
Texture2D g_TexMetalness0 : register( t5 );
Texture2D g_TexAmbientOcclusion0 : register( t6 );
Texture2D g_TexNormal0 : register( t7 );
Texture2D g_TexDisplacement0 : register( t8 );
Texture2D g_TexEmissivity0 : register( t9 );
Texture2D g_TexClearCoatNormal0 : register( t10 );
Texture2D g_TexBlendMask0 : register( t11 );

cbuffer MaterialEdition : register( b3 )
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

float3 ReadInput3D( in MaterialEditionInput materialInput, Texture2D textureSampler, sampler texSampler, float2 uvCoordinates, float3 defaultValue )
{
	float3 input = defaultValue;
	
	if ( materialInput.Type == INPUT_TYPE_1D ) {
		input = materialInput.Input1D.rrr;
	} else if ( materialInput.Type == INPUT_TYPE_3D ) {
		input = materialInput.Input3D.rgb;
	} else if ( materialInput.Type == INPUT_TYPE_TEXTURE ) {
		input = textureSampler.Sample( texSampler, uvCoordinates ).rgb;
	}
	
	return input;
}

float ReadInput1D( in MaterialEditionInput materialInput, Texture2D textureSampler, sampler texSampler, float2 uvCoordinates, float defaultValue )
{
    float input = defaultValue;
    
    if ( materialInput.Type == INPUT_TYPE_1D ) {
        input = materialInput.Input1D.r;
    } else if ( materialInput.Type == INPUT_TYPE_3D ) {
        input = materialInput.Input3D.r;
    } else if ( materialInput.Type == INPUT_TYPE_TEXTURE ) {
        input = textureSampler.Sample( texSampler, uvCoordinates ).r;
    }
    
    return input;
}

#define NYA_READ_LAYER( layerIdx )\
MaterialReadLayer ReadLayer##layerIdx( in VertexStageData VertexStage, in float3 N, in float3 V, in float3x3 TBNMatrix, inout bool needNormalMapUnpack, inout bool needSecondaryNormalMapUnpack )\
{\
	MaterialReadLayer layer;\
	\
	float2 uvCoords = ( VertexStage.uvCoord + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale;\
	\
    layer.BaseColor = ReadInput3D( g_Layers[layerIdx].BaseColor, g_TexBaseColor##layerIdx, g_BilinearSampler, uvCoords, float3( 0.42, 0.42, 0.42 ) );\
	if ( g_Layers[layerIdx].BaseColor.SamplingMode == SAMPLING_MODE_SRGB ) {\
        layer.BaseColor = accurateSRGBToLinear( layer.BaseColor );\
    }\
	layer.Roughness = ReadInput1D( g_Layers[layerIdx].Roughness, g_TexRoughness##layerIdx, g_BilinearSampler, uvCoords, 1.0f );\
    if ( g_Layers[layerIdx].Roughness.SamplingMode == SAMPLING_MODE_ALPHA_ROUGHNESS ) {\
        layer.Roughness = ( layer.Roughness * layer.Roughness );\
    }\
	layer.Metalness = ReadInput1D( g_Layers[layerIdx].Metalness, g_TexMetalness##layerIdx, g_BilinearSampler, uvCoords, 0.0f );\
    layer.AmbientOcclusion = ReadInput1D( g_Layers[layerIdx].AmbientOcclusion, g_TexAmbientOcclusion##layerIdx, g_BilinearSampler, uvCoords, 1.0f );\
	\
	layer.Emissivity = ReadInput1D( g_Layers[layerIdx].Emissivity, g_TexEmissivity##layerIdx, g_BilinearSampler, uvCoords, 0.0f );\
	layer.Reflectance = ReadInput1D( g_Layers[layerIdx].Reflectance, g_TexReflectance##layerIdx, g_BilinearSampler, uvCoords, 1.0f );\
	needNormalMapUnpack = ( g_Layers[layerIdx].Normal.Type == INPUT_TYPE_TEXTURE && g_Layers[layerIdx].Normal.SamplingMode == SAMPLING_MODE_TANGENT_SPACE );\
    if ( needNormalMapUnpack ) {\
        float4 sampledTexture = g_TexNormal##layerIdx.Sample( g_BilinearSampler, uvCoords );\
        layer.Normal = normalize( ( sampledTexture.rgb * g_Layers[layerIdx].NormalMapStrength ) * 2.0f - 1.0f );\
    } else {\
        layer.Normal = N;\
    }\
    needNormalMapUnpack = ( g_Layers[layerIdx].SecondaryNormal.Type == INPUT_TYPE_TEXTURE && g_Layers[layerIdx].SecondaryNormal.SamplingMode == SAMPLING_MODE_TANGENT_SPACE );\
    if ( needNormalMapUnpack ) {\
        float4 sampledTexture = g_TexClearCoatNormal##layerIdx.Sample( g_BilinearSampler, uvCoords );\
        layer.SecondaryNormal = normalize( ( sampledTexture.rgb * g_Layers[layerIdx].NormalMapStrength ) * 2.0f - 1.0f );\
    } else {\
        layer.SecondaryNormal = N;\
    }\
    layer.AlphaMask = ReadInput1D( g_Layers[layerIdx].AlphaMask, g_TexAlphaMask##layerIdx, g_BilinearSampler, uvCoords, 1.0f );\
    \
	layer.Refraction = g_Layers[layerIdx].Refraction;\
    layer.RefractionIor = g_Layers[layerIdx].RefractionIor;\
    layer.ClearCoat = g_Layers[layerIdx].ClearCoat;\
    layer.ClearCoatGlossiness = g_Layers[layerIdx].ClearCoatGlossiness;\
    \
    layer.DiffuseContribution = g_Layers[layerIdx].DiffuseContribution;\
    layer.SpecularContribution = g_Layers[layerIdx].SpecularContribution;\
    layer.NormalContribution = g_Layers[layerIdx].NormalContribution;\
    layer.AlphaCutoff = g_Layers[layerIdx].AlphaCutoff;\
    layer.SSStrength = g_Layers[layerIdx].SSStrength;\
	\
	layer.BlendMask = ReadInput1D( g_Layers[layerIdx].BlendMask, g_TexBlendMask##layerIdx, g_BilinearSampler, uvCoords, 1.0f );\
    \
	return layer;\
}\

NYA_READ_LAYER( 0 )

float4 EntryPointPS( in VertexStageData VertexStage ) : SV_TARGET0
{
    // Compute common terms from vertex stage variables
    float3 N = float3( 0, 1, 0 );
    float3 V = float3( 1, 1, 1 );
	static const float3x3 TBNMatrix = float3x3( 1, 0, 0, 0, 1, 0, 0, 0, 1 );
 
	bool needNormalMapUnpack = false, needSecondaryNormalMapUnpack = false;

	MaterialReadLayer layer = ReadLayer0( VertexStage, N, V, TBNMatrix, needNormalMapUnpack, needSecondaryNormalMapUnpack );

    return float4( layer.BaseColor, layer.AlphaMask );
}
