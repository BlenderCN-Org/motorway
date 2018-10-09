#include <Shared.h>
#include <Colormetry.hlsli>
#include <Sampling.hlsli>

cbuffer PerPass : register( b2 )
{
   uint2 g_BackbufferDimension;
};

// Pixel Stage Input Layouts
struct VertexStageData
{
    float4 position		: SV_POSITION;
    float4 positionWS   : POSITION0;
    float4 previousPosition : POSITION1;
    float3 normal       : NORMAL0;
    float depth         : DEPTH;
    float2 uvCoord      : TEXCOORD0;
#if PA_TERRAIN
    float3 positionMS   : POSITION2;
#endif
};

struct VertexDepthOnlyStageShaderData
{
    float4 position		: SV_POSITION;
    float2 uvCoord      : TEXCOORD0;
};

struct PixelStageData
{
    float4  Buffer0         : SV_TARGET0; // Shaded Surfaces Color
    float2  Buffer1         : SV_TARGET1; // Velocity (Opaque RenderList ONLY)
};

// BRDF independant inputs
TextureCubeArray    g_EnvProbeCaptureArray         : register( t5 );
TextureCubeArray    g_EnvProbeSpecularArray        : register( t8 );
Buffer<uint>        g_LightIndexBuffer             : register( t9 );
Texture2D           g_DFGLUTStandard               : register( t10 );
TextureCubeArray    g_EnvProbeDiffuseArray         : register( t12 );

Texture2DArray 		g_TerrainBaseColorReflectanceArray : register( t6 );
Texture2DArray 		g_TerrainNormalRoughnessArray : register( t7 );

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

sampler  g_BaseColorSampler                 : register( s0 );
sampler  g_ReflectanceSampler               : register( s1 );
sampler  g_RoughnessSampler                 : register( s2 );
sampler  g_MetalnessSampler                 : register( s3 );
sampler  g_AmbientOcclusionSampler          : register( s4 );
sampler  g_NormalMapSampler                 : register( s5 );
sampler  g_EmissivitySampler                : register( s6 );
sampler  g_AlphaMaskSampler                 : register( s7 );
sampler  g_SpecularEnvProbeSampler          : register( s8 );
SamplerComparisonState  g_ShadowMapSampler  : register( s15 );
sampler  g_LUTSampler                       : register( s10 );
sampler  g_DiffuseEnvProbeSampler           : register( s12 );

#if PA_TERRAIN
Texture2D<uint> g_TexSplatMap : register( t17 );
#else
// Layer0
Texture2D g_TexBaseColor0 : register( t17 );
Texture2D g_TexAlphaMask0 : register( t18 );
Texture2D g_TexReflectance0 : register( t19 );
Texture2D g_TexRoughness0 : register( t20 );
Texture2D g_TexMetalness0 : register( t21 );
Texture2D g_TexAmbientOcclusion0 : register( t22 );
Texture2D g_TexNormal0 : register( t23 );
Texture2D g_TexDisplacement0 : register( t24 );
Texture2D g_TexEmissivity0 : register( t25 );
Texture2D g_TexClearCoatNormal0 : register( t26 );

// Layer1
Texture2D g_TexBaseColor1 : register( t27 );
Texture2D g_TexAlphaMask1 : register( t28 );
Texture2D g_TexReflectance1 : register( t29 );
Texture2D g_TexRoughness1 : register( t30 );
Texture2D g_TexMetalness1 : register( t31 );
Texture2D g_TexAmbientOcclusion1 : register( t32 );
Texture2D g_TexNormal1 : register( t33 );
Texture2D g_TexDisplacement1 : register( t34 );
Texture2D g_TexEmissivity1 : register( t35 );
Texture2D g_TexClearCoatNormal1 : register( t36 );
Texture2D g_TexBlendMask1                : register( t37 );

// Layer2
Texture2D g_TexBaseColor2 : register( t38 );
Texture2D g_TexAlphaMask2 : register( t39 );
Texture2D g_TexReflectance2 : register( t40 );
Texture2D g_TexRoughness2 : register( t41 );
Texture2D g_TexMetalness2 : register( t42 );
Texture2D g_TexAmbientOcclusion2 : register( t43 );
Texture2D g_TexNormal2 : register( t44 );
Texture2D g_TexDisplacement2 : register( t45 );
Texture2D g_TexEmissivity2 : register( t46 );
Texture2D g_TexClearCoatNormal2 : register( t47 );
Texture2D g_TexBlendMask2                : register( t48 );
#endif

// Shading Model BRDF
#if PA_SHADING_MODEL_SHADING_MODEL_STANDARD
#include "ShadingModels/Standard.hlsl"
#elif PA_SHADING_MODEL_SHADING_MODEL_CLEAR_COAT
#include "ShadingModels/ClearCoat.hlsl"
#elif PA_SHADING_MODEL_SHADING_EMISSIVE
#include "ShadingModels/Emissive.hlsl"
#elif PA_SHADING_MODEL_SHADING_TILE_HEAT
#include "ShadingModels/TileHeat.hlsl"
#else
#include "ShadingModels/Debug.hlsl"
#endif

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
};

uint GetNumTilesX()
{
    return ( uint )( ( g_BackbufferDimension.x + TILE_RES - 1 ) / (float)TILE_RES );
}

uint GetNumTilesY()
{
    return ( uint )( ( g_BackbufferDimension.y + TILE_RES - 1 ) / (float)TILE_RES );
}

uint GetMaxNumLightsPerTile()
{
    uint clampedHeight = min( g_BackbufferDimension.y, 720 );
    return ( MAX_NUM_LIGHTS_PER_TILE - ( ADJUSTMENT_MULTIPLIER * ( clampedHeight / 120 ) ) );
}

uint GetTileIndex( float2 ScreenPos )
{
    float fTileRes = (float)TILE_RES;
    uint nNumCellsX = ( g_BackbufferDimension.x + TILE_RES - 1 ) / TILE_RES;
    uint nTileIdx = floor( ScreenPos.x / fTileRes ) + floor( ScreenPos.y / fTileRes ) * nNumCellsX;
    return nTileIdx;
}

float2 ApplyParallaxMapping( in float2 uvCoordinates, in float3 V, in float3 N, in float3 viewDirTS, in texture2D displacementMap, in float displacementStrength )
{
    // TODO Expose those parameters on the editor side
    static const int gMaxSampleCount = 64;
    static const int gMinSampleCount = 8;
        
    float2 maxParallaxOffset = -viewDirTS.xy*displacementStrength/viewDirTS.z;
    
    // Vary number of samples based on view angle between the eye and
    // the surface normal. (Head-on angles require less samples than
    // glancing angles.)
    int sampleCount = (int)lerp(gMaxSampleCount, gMinSampleCount, dot(-V, N));
    float zStep = 1.0f / (float)sampleCount;
    float2 texStep = maxParallaxOffset * zStep;
    
    // Precompute texture gradients since we cannot compute texture
    // gradients in a loop. Texture gradients are used to select the right
    // mipmap level when sampling textures. Then we use Texture2D.SampleGrad()
    // instead of Texture2D.Sample().
    float2 dx = ddx( uvCoordinates );
    float2 dy = ddy( uvCoordinates );
    int sampleIndex = 0;
    float2 currTexOffset = 0;
    float2 prevTexOffset = 0;
    float2 finalTexOffset = 0;
    float currRayZ = 1.0f - zStep;
    float prevRayZ = 1.0f;
    float currHeight = 0.0f;
    float prevHeight = 0.0f;
    
    // Ray trace the heightfield.
    while ( sampleIndex < sampleCount + 1 ) {
        currHeight = displacementMap.SampleGrad( g_NormalMapSampler, uvCoordinates + currTexOffset, dx, dy ).r;
        // Did we cross the height profile?
        if ( currHeight > currRayZ ) {
            // Do ray/line segment intersection and compute final tex offset.
            float t = (prevHeight - prevRayZ) / (prevHeight - currHeight + currRayZ - prevRayZ);
            finalTexOffset = prevTexOffset + t * texStep;
            
            // Exit loop.
            sampleIndex = sampleCount + 1;
        } else {
            ++sampleIndex;
            prevTexOffset = currTexOffset;
            prevRayZ = currRayZ;
            prevHeight = currHeight;
            currTexOffset += texStep;
            
            // Negative because we are going "deeper" into the surface.
            currRayZ -= zStep;
        }
    }
    
    return ( uvCoordinates + finalTexOffset );
}

#if PA_EDITOR

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

#ifndef PA_TERRAIN
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
    
#define READ_LAYER_FUNC( layerIdx )\
MaterialReadLayer ReadLayer##layerIdx( in VertexStageData VertexStage, in float3 N, in float3 V, in float3x3 TBNMatrix, inout bool needNormalMapUnpack, inout bool needSecondaryNormalMapUnpack )\

#define READ_LAYER_CONTENT( layerIdx )\
    MaterialReadLayer layer;\
    float2 uvCoordinates = VertexStage.uvCoord;\
    if ( g_Layers[layerIdx].Displacement.Type == INPUT_TYPE_TEXTURE ) {\
        float3 viewDirTS = normalize( mul( -V, transpose( TBNMatrix ) ) );\
        uvCoordinates = ApplyParallaxMapping( uvCoordinates, V, N, viewDirTS, g_TexDisplacement##layerIdx, g_Layers[layerIdx].DisplacementMapStrength );\
    }\
    layer.BaseColor = ReadInput3D( g_Layers[layerIdx].BaseColor, g_TexBaseColor##layerIdx, g_BaseColorSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale, float3( 0.42, 0.42, 0.42 ) );\
    if ( g_Layers[layerIdx].BaseColor.SamplingMode == SAMPLING_MODE_SRGB ) {\
        layer.BaseColor = accurateSRGBToLinear( layer.BaseColor );\
    }\
    layer.AlphaMask = ReadInput1D( g_Layers[layerIdx].AlphaMask, g_TexAlphaMask##layerIdx, g_AlphaMaskSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale, 1.0f );\
    layer.Reflectance = ReadInput1D( g_Layers[layerIdx].Reflectance, g_TexReflectance##layerIdx, g_ReflectanceSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale, 1.0f );\
    layer.Roughness = ReadInput1D( g_Layers[layerIdx].Roughness, g_TexRoughness##layerIdx, g_RoughnessSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale, 1.0f );\
    if ( g_Layers[layerIdx].Roughness.SamplingMode == SAMPLING_MODE_ALPHA_ROUGHNESS ) {\
        layer.Roughness = ( layer.Roughness * layer.Roughness );\
    }\
    layer.Metalness = ReadInput1D( g_Layers[layerIdx].Metalness, g_TexMetalness##layerIdx, g_MetalnessSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale, 0.0f );\
    layer.AmbientOcclusion = ReadInput1D( g_Layers[layerIdx].AmbientOcclusion, g_TexAmbientOcclusion##layerIdx, g_AmbientOcclusionSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale, 1.0f );\
    layer.Emissivity = ReadInput1D( g_Layers[layerIdx].Emissivity, g_TexEmissivity##layerIdx, g_EmissivitySampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale, 0.0f );\
    \
    needSecondaryNormalMapUnpack = ( g_Layers[layerIdx].SecondaryNormal.Type == INPUT_TYPE_TEXTURE && g_Layers[layerIdx].SecondaryNormal.SamplingMode == SAMPLING_MODE_TANGENT_SPACE );\
    if ( needSecondaryNormalMapUnpack ) {\
        float4 sampledTexture = g_TexClearCoatNormal##layerIdx.Sample( g_NormalMapSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale );\
        layer.SecondaryNormal = normalize( ( sampledTexture.rgb * g_Layers[layerIdx].SecondaryNormalMapStrength ) * 2.0f - 1.0f );\
    } else {\
        layer.SecondaryNormal = ReadInput3D( g_Layers[layerIdx].SecondaryNormal, g_TexClearCoatNormal##layerIdx, g_NormalMapSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale, N );\
    }\
    \
    needNormalMapUnpack = ( g_Layers[layerIdx].Normal.Type == INPUT_TYPE_TEXTURE && g_Layers[layerIdx].Normal.SamplingMode == SAMPLING_MODE_TANGENT_SPACE );\
    if ( needNormalMapUnpack ) {\
        float4 sampledTexture = g_TexNormal##layerIdx.Sample( g_NormalMapSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale );\
        layer.Normal = normalize( ( sampledTexture.rgb * g_Layers[layerIdx].NormalMapStrength ) * 2.0f - 1.0f );\
    } else {\
        layer.Normal = ReadInput3D( g_Layers[layerIdx].Normal, g_TexNormal##layerIdx, g_NormalMapSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale, N );\
    }\
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
    \

#define READ_LAYER_BLEND_MASK( layerIdx )\
        layer.BlendMask = ReadInput1D( g_Layers[layerIdx].BlendMask, g_TexBlendMask##layerIdx, g_AlphaMaskSampler, ( uvCoordinates + g_Layers[layerIdx].LayerOffset ) * g_Layers[layerIdx].LayerScale, 1.0f );\
        
#define READ_LAYER_SKIP_BLEND_MASK( layerIdx )\
    layer.BlendMask = 1.0f;

#define READ_LAYER_FUNC_END( layerIdx )\
    return layer;

READ_LAYER_FUNC( 0 )
{
    READ_LAYER_CONTENT( 0 )
    READ_LAYER_SKIP_BLEND_MASK( 0 )
    READ_LAYER_FUNC_END( 0 )
}
    
READ_LAYER_FUNC( 1 )
{
    READ_LAYER_CONTENT( 1 )
    READ_LAYER_BLEND_MASK( 1 )
    READ_LAYER_FUNC_END( 1 )
}

READ_LAYER_FUNC( 2 )
{
    READ_LAYER_CONTENT( 2 )
    READ_LAYER_BLEND_MASK( 2 )
    READ_LAYER_FUNC_END( 2 )
}
#endif
#endif

float3x3 compute_tangent_frame(float3 N, float3 P, float2 UV, out float3 T, out float3 B)
{
	// ddx_coarse can be faster than ddx, but could result in artifacts. Haven't observed any artifacts yet.
	float3 dp1 = ddx_coarse(P);
	float3 dp2 = ddy_coarse(P);
	float2 duv1 = ddx_coarse(UV);
	float2 duv2 = ddy_coarse(UV);

	float3x3 M = float3x3(dp1, dp2, cross(dp1, dp2));
	float2x3 inverseM = float2x3(cross(M[1], M[2]), cross(M[2], M[0]));
	T = normalize(mul(float2(duv1.x, duv2.x), inverseM));
	B = normalize(mul(float2(duv1.y, duv2.y), inverseM));

	return float3x3(T, B, N);
}

uint GetEntityCountForCurrentTile( float2 positionScreenSpace )
{
    // Sum non culled lights first
    uint tileHeat = DirectionalLightCount + SphereLightCount + DiscLightCount + RectangleAndTubeLightCount + GlobalEnvironmentProbeCount;

    // Retrieve current tile index
    uint tileIndex = GetTileIndex( positionScreenSpace );
    uint maxNumLightPerTile = GetMaxNumLightsPerTile();

    uint scaledTileIndex = tileIndex * maxNumLightPerTile;
    uint nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];

    // Point Lights
    [loop]
    while ( nNextLightIndex != 0x7fffffff ) {
        uint nLightIndex = nNextLightIndex;
        scaledTileIndex++;
        nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];
        tileHeat++;
    }

    scaledTileIndex++;
    nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];

    // Spot Lights
    [loop]
    while ( nNextLightIndex != 0x7fffffff ) {
        uint nLightIndex = nNextLightIndex;
        scaledTileIndex++;
        nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];
        tileHeat++;
    }
    
    scaledTileIndex++;
    nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];
    
    [loop]  
    while ( nNextLightIndex != 0x7fffffff ) {
        uint nLightIndex = nNextLightIndex;
        scaledTileIndex++;
        nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];
        tileHeat++;
    }
    
    return tileHeat;
}

static const float3 kRadarColors[14] = 
{
    {0,0.9255,0.9255},   // cyan
    {0,0.62745,0.9647},  // light blue
    {0,0,0.9647},        // blue
    {0,1,0},             // bright green
    {0,0.7843,0},        // green
    {0,0.5647,0},        // dark green
    {1,1,0},             // yellow
    {0.90588,0.75294,0}, // yellow-orange
    {1,0.5647,0},        // orange
    {1,0,0},             // bright red
    {0.8392,0,0},        // red
    {0.75294,0,0},       // dark red
    {1,0,1},             // magenta
    {0.6,0.3333,0.7882}, // purple
};

float3 BlendNormals_UDN( in float3 baseNormal, in float3 topNormal )
{
    static const float3 c = float3( 2, 1, 0 );  
    float3 blendedNormals = topNormal * c.yyz + baseNormal.xyz;
    return normalize( blendedNormals );
}

void BlendLayers( inout MaterialReadLayer baseLayer, in MaterialReadLayer nextLayer )
{
    baseLayer.BaseColor = lerp( baseLayer.BaseColor, nextLayer.BaseColor, nextLayer.BlendMask * nextLayer.DiffuseContribution );
    baseLayer.Reflectance = lerp( baseLayer.Reflectance, nextLayer.Reflectance, nextLayer.BlendMask * nextLayer.SpecularContribution );
    
    baseLayer.Roughness = lerp( baseLayer.Roughness, nextLayer.Roughness, nextLayer.BlendMask * nextLayer.SpecularContribution );
    baseLayer.Metalness = lerp( baseLayer.Metalness, nextLayer.Metalness, nextLayer.BlendMask * nextLayer.SpecularContribution );
    baseLayer.AmbientOcclusion = lerp( baseLayer.AmbientOcclusion, nextLayer.AmbientOcclusion, nextLayer.BlendMask );
    baseLayer.Emissivity = lerp( baseLayer.Emissivity, nextLayer.Emissivity, nextLayer.BlendMask );
    baseLayer.Normal = BlendNormals_UDN( baseLayer.Normal, nextLayer.Normal * nextLayer.NormalContribution * nextLayer.BlendMask );
        
    baseLayer.AlphaMask = lerp( baseLayer.AlphaMask, nextLayer.AlphaMask, nextLayer.BlendMask );
    
    baseLayer.SecondaryNormal = nextLayer.SecondaryNormal * nextLayer.NormalContribution;
    baseLayer.BlendMask = nextLayer.BlendMask;
    
    baseLayer.Refraction = lerp( baseLayer.Refraction, nextLayer.Refraction, nextLayer.BlendMask * nextLayer.SpecularContribution );
    baseLayer.RefractionIor = lerp( baseLayer.RefractionIor, nextLayer.RefractionIor, nextLayer.BlendMask );   
    baseLayer.ClearCoat = nextLayer.ClearCoat;
    baseLayer.ClearCoatGlossiness = nextLayer.ClearCoatGlossiness;
    
    baseLayer.DiffuseContribution = nextLayer.DiffuseContribution;
    baseLayer.SpecularContribution = nextLayer.SpecularContribution;
    baseLayer.NormalContribution = nextLayer.NormalContribution;
    baseLayer.AlphaCutoff = nextLayer.AlphaCutoff;
}

#define DFG_TEXTURE_SIZE 512.0f
#define LD_MIP_COUNT 8

void EvaluateIBL(
    inout float3 diffuseSum,
    inout float3 specularSum,
    in uint nNextLightIndex,
    in int scaledTileIndex,
    in float NoV,
    in float roughness,
    in float linearRoughness,
    in float3 N,
    in float3 V,
    in float3 R,
    in float3 positionWS,
    in float ambientOcclusion,
    in float3 albedo,
    in float3 fresnelColor,
    in float f90
)
{
    // Reflections / IBL
    // Rebuild the function
    // L . D . ( f0.Gv.(1-Fc) + Gv.Fc ) . cosTheta / (4 . NdotL . NdotV )
    float dfgNoV = max( NoV, 0.5f / DFG_TEXTURE_SIZE );
    float3 dfgLUTSample = g_DFGLUTStandard.SampleLevel( g_LUTSampler, float2( dfgNoV, roughness ), 0 ).rgb;
    float diffF = dfgLUTSample.z;
    float2 preDFG = dfgLUTSample.xy;

    float3 dominantN = getDiffuseDominantDir( N, V, dfgNoV, roughness );
    float3 dominantR = getSpecularDominantDir( N, R, roughness );

    float mipLevel = linearRoughnessToMipLevel( linearRoughness, LD_MIP_COUNT );

    diffuseSum = float3( 0, 0, 0 );
    specularSum = float3( 0, 0, 0 );

    for ( uint i = 0; i < GlobalEnvironmentProbeCount; i++ ) {
        diffuseSum.rgb = g_EnvProbeDiffuseArray.Sample( g_DiffuseEnvProbeSampler, float4( dominantN, GlobalEnvProbes[i].Index ) ).rgb;
        specularSum.rgb = g_EnvProbeSpecularArray.SampleLevel( g_SpecularEnvProbeSampler, float4( dominantR, GlobalEnvProbes[i].Index ), mipLevel ).rgb;
    }

     float4 localEnvProbeSumDiff = float4( 0, 0, 0, 0 );
     float4 localEnvProbeSumSpec = float4( 0, 0, 0, 0 );

     [loop]
     while ( nNextLightIndex != 0x7fffffff ) {
         uint nLightIndex = nNextLightIndex;
         scaledTileIndex++;
         nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];

         float3 clipSpacePos = mul( float4( positionWS, 1.0f ), LocalEnvProbes[nLightIndex].InverseModelMatrix ).xyz;
         float3 uvw = clipSpacePos.xyz*float3( 0.5f, -0.5f, 0.5f ) + 0.5f;

         [branch]
         if ( !any( uvw - saturate( uvw ) ) ) {
             // Perform parallax correction of reflection ray (R) into OBB:
             float3 RayLS = mul( dominantR, ( float3x3 )LocalEnvProbes[nLightIndex].InverseModelMatrix );
             float3 FirstPlaneIntersect = ( float3( 1, 1, 1 ) - clipSpacePos ) / RayLS;
             float3 SecondPlaneIntersect = ( -float3( 1, 1, 1 ) - clipSpacePos ) / RayLS;
             float3 FurthestPlane = max( FirstPlaneIntersect, SecondPlaneIntersect );
             float Distance = min( FurthestPlane.x, min( FurthestPlane.y, FurthestPlane.z ) );
             float3 IntersectPositionWS = positionWS + dominantR * Distance;
             float3 R_parallaxCorrected = IntersectPositionWS - LocalEnvProbes[nLightIndex].PositionAndRadiusWorldSpace.xyz;

             float distRough = computeDistanceBaseRoughness(
                 length( positionWS - IntersectPositionWS ),
                 length( LocalEnvProbes[nLightIndex].PositionAndRadiusWorldSpace.xyz - IntersectPositionWS ),
                 linearRoughness );

             // Recompute mip for parallax corrected environment probes
             float mip = 1.0 - 1.2 * log2( distRough );
             mip = LD_MIP_COUNT - 1.0 - mip;

             float4 envMapDiff = g_EnvProbeDiffuseArray.Sample( g_DiffuseEnvProbeSampler, float4( dominantN, LocalEnvProbes[nLightIndex].Index ) );
             float4 envMapSpec = g_EnvProbeSpecularArray.SampleLevel( g_SpecularEnvProbeSampler, float4( R_parallaxCorrected, LocalEnvProbes[nLightIndex].Index ), mip );

             float edgeBlend = 1 - pow( saturate( max( abs( clipSpacePos.x ), max( abs( clipSpacePos.y ), abs( clipSpacePos.z ) ) ) ), 8 );

             envMapDiff.a = edgeBlend;
             envMapSpec.a = edgeBlend;

             // Perform probe blending
             localEnvProbeSumDiff.rgb = ( 1 - localEnvProbeSumDiff.a ) * ( envMapDiff.a * envMapDiff.rgb ) + localEnvProbeSumDiff.rgb;
             localEnvProbeSumDiff.a = envMapDiff.a + ( 1 - envMapDiff.a ) * localEnvProbeSumDiff.a;

             localEnvProbeSumSpec.rgb = ( 1 - localEnvProbeSumSpec.a ) * ( envMapSpec.a * envMapSpec.rgb ) + localEnvProbeSumSpec.rgb;
             localEnvProbeSumSpec.a = envMapSpec.a + ( 1 - envMapSpec.a ) * localEnvProbeSumSpec.a;

             // if the accumulation reached 1, we skip the rest of the probes:
             if ( localEnvProbeSumDiff.a >= 1.0f ) {
                 break;
             }
         }
     }

     diffuseSum.rgb = lerp( diffuseSum.rgb, localEnvProbeSumDiff.rgb, localEnvProbeSumDiff.a );
     specularSum.rgb = lerp( specularSum.rgb, localEnvProbeSumSpec.rgb, localEnvProbeSumSpec.a );
    
     // Compute final terms
     diffuseSum.rgb = ( diffuseSum.rgb * diffF ) * albedo;
     specularSum.rgb = ( specularSum.rgb * ( fresnelColor * preDFG.x + f90 * preDFG.y ) ) * computeSpecOcclusion( NoV, ambientOcclusion, linearRoughness ); // LD.(f0.Gv.(1 - Fc) + Gv.Fc.f90)
}

// Baked layers read (see material build python script)
#ifndef PA_EDITOR
FLAN_LAYERS_READ
#endif

#if PA_TERRAIN
MaterialReadLayer FetchTerrainMaterial( const float3 positionMS, const float2 texCoordinates, const float3 N )
{
	uint splatIndex = g_TexSplatMap[positionMS.xz];
	
	// Retrieve texture coordinates
	float4 samplingParameters = g_TerrainMaterials[splatIndex].samplingParameters; // xy offset; zw scale
	float2 samplingCoordinates = ( texCoordinates.xy + samplingParameters.xy ) * samplingParameters.zw;
		
	uint streamedTextureIndex = g_TerrainMaterials[splatIndex].splatIndex;
		
	float4 normalAndRoughness = g_TerrainNormalRoughnessArray.Sample( g_BaseColorSampler, float3( samplingCoordinates.xy, streamedTextureIndex ) ); //TriplanarSample3D( g_TerrainNormalRoughnessArray, g_BaseColorSampler, streamedTextureIndex, positionMS, N );
	float3 baseColor = g_TerrainBaseColorReflectanceArray.Sample( g_BaseColorSampler, float3( samplingCoordinates.xy, streamedTextureIndex ) ).rgb;
	
	//TriplanarSample3D( g_TerrainBaseColorReflectanceArray, g_BaseColorSampler, streamedTextureIndex, positionMS, normalAndRoughness.rgb );
	
	MaterialReadLayer layer;
	layer.BaseColor = accurateSRGBToLinear( baseColor );
	layer.Reflectance = 0.50f; // baseColor.a;	
	layer.Roughness = normalAndRoughness.a;
	layer.Metalness = 0.0f;
	layer.AmbientOcclusion = 1.0f;
	layer.Emissivity = 0.0f;
	layer.Normal = normalize( normalAndRoughness.rgb * 2.0f - 1.0f );
	layer.AlphaMask = 1.0f;
	layer.SecondaryNormal = N;
	layer.BlendMask = 1.0f;
	layer.Refraction = 0.0f;
	layer.RefractionIor = 0.0f;
	layer.ClearCoat = 0.0f;
	layer.ClearCoatGlossiness = 0.0f;
	layer.DiffuseContribution = 1.00f;
	layer.SpecularContribution = 1.00f;
	layer.NormalContribution = 1.00f;
	layer.AlphaCutoff = 0.0f;
	
	return layer;
}
#endif

PixelStageData EntryPointPS( VertexStageData VertexStage, bool isFrontFace : SV_IsFrontFace )
{
    // Compute common terms from vertex stage variables
    float3 N = normalize( VertexStage.normal );
    float3 V = normalize( WorldPosition.xyz - VertexStage.positionWS.xyz );
    
    float3 t, b;
    float3x3 TBNMatrix = compute_tangent_frame( N, VertexStage.positionWS.xyz, VertexStage.uvCoord, t, b );

#if PA_EDITOR
#if PA_TERRAIN
	MaterialReadLayer BaseLayer = FetchTerrainMaterial( VertexStage.positionMS, VertexStage.uvCoord.xy, N );
	
	MaterialReadLayer eastFetch = FetchTerrainMaterial( VertexStage.positionMS + float3( 1, 0, 0 ), VertexStage.uvCoord.xy, N );
	eastFetch.BlendMask = 0.15;
	BlendLayers( BaseLayer, eastFetch );
	
	MaterialReadLayer southEastFetch = FetchTerrainMaterial( VertexStage.positionMS + float3( 1, 0, 1 ), VertexStage.uvCoord.xy, N );
	southEastFetch.BlendMask = 0.15;	
	BlendLayers( BaseLayer, southEastFetch );
	
	MaterialReadLayer southFetch = FetchTerrainMaterial( VertexStage.positionMS + float3( 0, 0, 1 ), VertexStage.uvCoord.xy, N );
	southFetch.BlendMask = 0.15;	
	BlendLayers( BaseLayer, southFetch );
	
	BaseLayer.Normal = normalize( mul( BaseLayer.Normal, TBNMatrix ) );
#else
    bool needNormalMapUnpack0 = false, needNormalMapUnpack1 = false, needNormalMapUnpack2 = false;
    bool needSecondaryNormalMapUnpack0 = false, needSecondaryNormalMapUnpack1 = false, needSecondaryNormalMapUnpack2 = false;
    
    MaterialReadLayer BaseLayer = ReadLayer0( VertexStage, N, V, TBNMatrix, needNormalMapUnpack0, needSecondaryNormalMapUnpack0 );
    
    // NOTE Only use the double branch for material realtime edition
    // Otherwise, resolve branches offline at compile time
    if ( g_LayerCount > 1 ) {
        MaterialReadLayer layer1 = ReadLayer1( VertexStage, N, V, TBNMatrix, needNormalMapUnpack1, needSecondaryNormalMapUnpack1 );
        BlendLayers( BaseLayer, layer1 );
        
        if ( g_LayerCount > 2 ) {
            MaterialReadLayer layer2 = ReadLayer2( VertexStage, N, V, TBNMatrix, needNormalMapUnpack2, needSecondaryNormalMapUnpack2 );
            BlendLayers( BaseLayer, layer2 );
        }
    }
  
    const bool needUnpackAndTBNMult = ( needNormalMapUnpack0 || needNormalMapUnpack1 || needNormalMapUnpack2 );  
    if ( needUnpackAndTBNMult ) {
        BaseLayer.Normal = normalize( mul( BaseLayer.Normal, TBNMatrix ) );
    }
  
    const bool needUnpackAndTBNMultSecondary = ( needSecondaryNormalMapUnpack0 || needSecondaryNormalMapUnpack1 || needSecondaryNormalMapUnpack2 );
    if ( needUnpackAndTBNMult ) {
        BaseLayer.SecondaryNormal = normalize( mul( BaseLayer.SecondaryNormal, TBNMatrix ) );
    }
#endif 
#if FLAN_DEBUG_TILE_HEAT
    uint entityCount = GetEntityCountForCurrentTile( VertexStage.position.xy );
    
    uint dbgmaxNumLightPerTile = GetMaxNumLightsPerTile();
    
    uint2 screenPos = uint2( VertexStage.position.x, VertexStage.position.y );

    if ( entityCount == 0 
    || ( screenPos.x % TILE_RES ) == 0 
    || ( screenPos.y % TILE_RES ) == 0 ) {
        BaseLayer.BaseColor = float3(0,0,0);
    } else if( entityCount == dbgmaxNumLightPerTile ) {
        BaseLayer.BaseColor = float3( 0.847, 0.745, 0.921 );
    } else if ( entityCount > dbgmaxNumLightPerTile ) {
        BaseLayer.BaseColor = float3( 1, 1, 1 );
    } else {
        float fLogBase = exp2(0.07142857f*log2((float)dbgmaxNumLightPerTile));
        uint nColorIndex = floor(log2((float)entityCount) / log2(fLogBase));
        BaseLayer.BaseColor = kRadarColors[nColorIndex];
    }  
#endif
#else
	FLAN_BUILD_LAYERS
#endif
  
	N = BaseLayer.Normal;
	
#if PA_EDITOR
    // Flip the normal for the backface
    if ( g_IsDoubleFace == 1 && isFrontFace ) {
#elif PA_IS_DOUBLE_FACE
    if ( !isFrontFace ) {
#endif
        N = N * float3( -1, -1, 1 );
    }

    float3 R = reflect( -V, N );
    
    const LightSurfaceInfos surface = {
        V,
        BaseLayer.AmbientOcclusion,

        BaseLayer.Normal,
        BaseLayer.Roughness,

        // Albedo
        lerp( BaseLayer.BaseColor, 0.0f, BaseLayer.Metalness ),
        BaseLayer.Metalness,

        // F0
        lerp( ( 0.16f * ( BaseLayer.Reflectance * BaseLayer.Reflectance ) ), BaseLayer.BaseColor, BaseLayer.Metalness ),

        // F90
        saturate( 50.0 * dot( BaseLayer.Reflectance, 0.33 ) ),

        VertexStage.positionWS.xyz,

        // LinearRoughness
        max( 0.01f, ( BaseLayer.Roughness * BaseLayer.Roughness ) ),

        // Reflection Vector
        R,

        // Disney BRDF LinearRoughness
        ( 0.5f + BaseLayer.Roughness / 2.0f ),

        // Clear Coat Normal
        BaseLayer.SecondaryNormal,

        // N dot V
        // FIX Use clamp instead of saturate (clamp dot product between epsilon and 1)
        clamp( dot( BaseLayer.Normal, V ), 1e-5f, 1.0f ),

        BaseLayer.ClearCoat,
        BaseLayer.ClearCoatGlossiness,

        // Explicit Padding
        float2( 0.0f, 0.0f )
    };

    // Lighting
    float4 LightContribution = float4( 0, 0, 0, 1 );
    float3 L = float3( 0, 0, 0 );
    
#if PA_SHADING_MODEL_SHADING_EMISSIVE
    LightContribution.rgb += DoShading( L, surface );
    LightContribution.rgb += ( LightContribution.rgb * BaseLayer.Emissivity );
#else
    // Directional Lights
    [loop]
    for ( uint i = 0; i < DirectionalLightCount; ++i ) {
        float3 directionalLightIlluminance = GetDirectionalLightIlluminance( DirectionalLights[i], surface, VertexStage.depth, L );
        LightContribution.rgb += DoShading( L, surface ) * directionalLightIlluminance;
    }

    // Sphere Lights
    [loop]
    for ( uint j = 0; j < SphereLightCount; ++j ) {
        float3 sphereLightIlluminance = GetSphereLightIlluminance( SphereLights[j], surface, VertexStage.depth, L );
        LightContribution.rgb += DoShading( L, surface ) * sphereLightIlluminance;
    }

    // Disc Lights
    [loop]
    for ( uint k = 0; k < DiscLightCount; ++k ) {
        float3 discLightIlluminance = GetDiscLightIlluminance( DiscLights[k], surface, VertexStage.depth, L );
        LightContribution.rgb += DoShading( L, surface ) * discLightIlluminance;
    }

    // Rectangle And Tube Lights
    [loop]
    for ( uint l = 0; l < RectangleAndTubeLightCount; ++l ) {
        float3 rectangleOrTubeLightIlluminance = float3( 0, 0, 0 );

        //if ( RectangleAndTubeLights[l].IsTubeLight == 1 ) {
            rectangleOrTubeLightIlluminance = GetTubeLightIlluminance( RectangleAndTubeLights[l], surface, VertexStage.depth, L );
        // } else {
            // rectangleOrTubeLightIlluminance = GetRectangleLightIlluminance( RectangleAndTubeLights[l], surface, VertexStage.depth, L );
        // }

        LightContribution.rgb += DoShading( L, surface ) * rectangleOrTubeLightIlluminance;
    }

    // Retrieve current tile index
    uint tileIndex = GetTileIndex( VertexStage.position.xy );
    uint maxNumLightPerTile = GetMaxNumLightsPerTile();

    uint scaledTileIndex = tileIndex * maxNumLightPerTile;
    uint nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];

    // Point Lights
    [loop]
    while ( nNextLightIndex != 0x7fffffff ) {
        uint nLightIndex = nNextLightIndex;
        scaledTileIndex++;
        nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];

        float3 pointLightIlluminance = GetPointLightIlluminance( PointLights[nLightIndex], surface, VertexStage.depth, L );
        LightContribution.rgb += DoShading( L, surface ) * pointLightIlluminance;
    }

    scaledTileIndex++;
    nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];

    // Spot Lights
    [loop]
    while ( nNextLightIndex != 0x7fffffff ) {
        uint nLightIndex = nNextLightIndex;
        scaledTileIndex++;
        nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];

        float3 spotLightIlluminance = GetSpotLightIlluminance( SpotLights[nLightIndex], surface, VertexStage.depth, L );
        LightContribution.rgb += DoShading( L, surface ) * spotLightIlluminance;
    }
  
    // Emissive surface
    LightContribution.rgb += ( LightContribution.rgb * BaseLayer.Emissivity );
    
    scaledTileIndex++;
    nNextLightIndex = g_LightIndexBuffer[scaledTileIndex];

#ifndef PA_PROBE_CAPTURE
#if PA_SHADING_MODEL_SHADING_MODEL_STANDARD
    float3 diffuseSum, specularSum;
    EvaluateIBL( diffuseSum, specularSum, nNextLightIndex, scaledTileIndex, surface.NoV, surface.Roughness, surface.LinearRoughness, surface.N, surface.V, surface.R, surface.PositionWorldSpace, surface.AmbientOcclusion, surface.Albedo, surface.FresnelColor, surface.F90 );

    // Add indirect contribution to output
    LightContribution.rgb += ( diffuseSum.rgb + specularSum.rgb );
#elif PA_SHADING_MODEL_SHADING_MODEL_CLEAR_COAT
    float3 IndirectDiffuse, SpecularReflections;
    EvaluateIBL( IndirectDiffuse, SpecularReflections, nNextLightIndex, scaledTileIndex, surface.NoV, surface.Roughness, surface.LinearRoughness, surface.N, surface.V, surface.R, surface.PositionWorldSpace, surface.AmbientOcclusion, surface.Albedo, surface.FresnelColor, surface.F90 );

    // Compute Specular Reflections for the ClearCoat Layer
    // IOR = 1.5 -> F0 = 0.04
    float ClearCoatF0 = 0.04f;
    float ClearCoatRoughness = 1.0f - surface.ClearCoatGlossiness;
    float ClearCoatLinearRoughness = ( ClearCoatRoughness * ClearCoatRoughness );
    float ClearCoatNoV = saturate( dot( surface.V, surface.SecondaryNormal ) ) + 1e-5f;

    float Fc = pow( 1 - ClearCoatNoV, 5.0f );
    float ClearCoatFresnel = Fc + ( 1 - Fc ) * ClearCoatF0;
    ClearCoatFresnel *= surface.ClearCoat;

    float3 ClearCoatR = reflect( -surface.V, surface.SecondaryNormal );

    float LightTransmitAmt = ( 1.0f - ClearCoatFresnel );

    // Accentuate rim lighting (which is usually strong on clearcoat-like paints)
    float3 diffuseSum, ClearCoatSpecular;
    EvaluateIBL( diffuseSum, ClearCoatSpecular, nNextLightIndex, scaledTileIndex, surface.NoV, ClearCoatRoughness, ClearCoatLinearRoughness, surface.SecondaryNormal, surface.V, ClearCoatR, surface.PositionWorldSpace, surface.AmbientOcclusion, float3( 0, 0, 0 ), ClearCoatF0, ( 1.0f - surface.ClearCoat ) );

    float Mip = linearRoughnessToMipLevel( ClearCoatLinearRoughness, 8 );
    float3 ClearCoatEnvironment = g_EnvProbeCaptureArray.SampleLevel( g_DiffuseEnvProbeSampler, float4( ClearCoatR, 0 ), Mip ).xyz;
    
    float3 iblClearCoat = ( IndirectDiffuse + ( ( SpecularReflections * LightTransmitAmt ) + ClearCoatSpecular ) ) + ( ClearCoatEnvironment * ClearCoatFresnel );

    LightContribution.rgb += iblClearCoat;
#endif
#endif
  
    // Refraction
#if PA_EDITOR
#define PA_USE_REFRACTION 1
    if ( BaseLayer.Refraction > 0.0f ) {
#endif

#if PA_USE_REFRACTION
        float3 I = normalize( VertexStage.positionWS.xyz - WorldPosition.xyz );
        float NoI = saturate( dot( surface.N, I ) );
        float Fc = pow( 1 - NoI, 5.0f );
        float RefractionFresnel = Fc + ( 1 - Fc ) * surface.FresnelColor.r;
        float refractionIor = lerp( 1.0f, BaseLayer.RefractionIor, RefractionFresnel );
        float refractionRatio = 1.00f / BaseLayer.RefractionIor;
        float3 RefractionVector = refract( V, surface.N, refractionRatio );
        
        float3 refractionDiffuse, refractionSpecular;
        EvaluateIBL( refractionDiffuse, refractionSpecular, nNextLightIndex, scaledTileIndex, surface.NoV, surface.Roughness, surface.LinearRoughness, surface.N, surface.V, RefractionVector, surface.PositionWorldSpace, surface.AmbientOcclusion, surface.Albedo, surface.FresnelColor, surface.F90 );

        LightContribution.rgb += ( refractionSpecular * BaseLayer.Refraction );
#endif

#if PA_EDITOR    
    }
#endif

#ifndef PA_PROBE_CAPTURE
    // Atmospheric Scattering Contribution
	float3 atmosphereTransmittance = float3( 0, 0, 0 );

    float3 atmosphereSamplingPos = float3( WorldPosition.x, WorldPosition.z, max( WorldPosition.y, 0.0f )  ) * 0.001f;
    float3 atmosphereVertexPos = float3( VertexStage.positionWS.x, VertexStage.positionWS.z,  max( VertexStage.positionWS.y, 0.0f ) ) * 0.001f;
      
	float3 atmosphereInScatter = GetSkyRadianceToPoint
	( 
		atmosphereSamplingPos - g_EarthCenter,
		atmosphereVertexPos - g_EarthCenter,
		0.001f,
		g_SunDirection,
		atmosphereTransmittance 
	);

	LightContribution.rgb = LightContribution.rgb + atmosphereInScatter;
#endif

    // PA_ENABLE_ALPHA_BLEND
#if PA_EDITOR
    if ( g_EnableAlphaBlend == 1 ) {
#define PA_ENABLE_ALPHA_BLEND 1
#endif

#if PA_ENABLE_ALPHA_BLEND
        LightContribution.a = BaseLayer.AlphaMask;
        
        // Premultiply alpha
        LightContribution.rgb *= LightContribution.a;
#endif

#if PA_EDITOR
    }
#endif


    // PA_ENABLE_ALPHA_TO_COVERAGE
#if PA_EDITOR
    if ( g_EnableAlphaToCoverage == 1 ) {
#define PA_ENABLE_ALPHA_TO_COVERAGE 1
#endif

#if PA_ENABLE_ALPHA_TO_COVERAGE
		// Sharpen cutoff
        LightContribution.a = ( LightContribution.a - BaseLayer.AlphaCutoff ) / max( fwidth( LightContribution.a ), 0.0001f ) + 0.5f;
#endif

#if PA_EDITOR
    }
#endif    
#endif

	// PA_WRITE_VELOCITY
    float2 Velocity = float2( 0, 0 );

#if PA_EDITOR
    if ( g_WriteVelocity == 1 ) {
#define PA_WRITE_VELOCITY 1
#endif

#if PA_WRITE_VELOCITY
        float2 prevPositionSS = (VertexStage.previousPosition.xy / VertexStage.previousPosition.z) * float2(0.5f, -0.5f) + 0.5f;
        prevPositionSS *= float2( g_BackbufferDimension );
       
        Velocity = VertexStage.position.xy - prevPositionSS;
        Velocity -= g_CameraJitteringOffset;
#endif
		
#if PA_EDITOR
    }
#endif
    
    // Write output to buffer(s)
	PixelStageData output;
	output.Buffer0 = LightContribution;
	output.Buffer1 = Velocity;
	
	return output;
}

void BlendDepthLayers( in float blendMask, inout float alphaMask, inout float alphaCutoff, in float nextAlphaMask, in float nextAlphaCutoff )
{
    alphaMask = lerp( alphaMask, nextAlphaMask, blendMask );
    alphaCutoff = lerp( alphaCutoff, nextAlphaCutoff, blendMask );
}

void EntryPointDepthPS( in VertexDepthOnlyStageShaderData VertexStage )
{
#if PA_EDITOR
#ifndef PA_TERRAIN
#define PA_ENABLE_ALPHA_TEST 1
    bool needNormalMapUnpack0 = false, needNormalMapUnpack1 = false, needNormalMapUnpack2 = false;
    bool needSecondaryNormalMapUnpack0 = false, needSecondaryNormalMapUnpack1 = false, needSecondaryNormalMapUnpack2 = false;
    
    float alphaCutoff = g_Layers[0].AlphaCutoff;
    float alphaMask = ReadInput1D( g_Layers[0].AlphaMask, g_TexAlphaMask0, g_AlphaMaskSampler, ( VertexStage.uvCoord + g_Layers[0].LayerOffset ) * g_Layers[0].LayerScale, 1.0f );
    
    // NOTE Only use the double branch for material realtime edition
    // Otherwise, resolve branches offline at compile time
    if ( g_LayerCount > 1 ) {
        float alphaCutoff1 = g_Layers[1].AlphaCutoff;
        float alphaMask1 = ReadInput1D( g_Layers[1].AlphaMask, g_TexAlphaMask1, g_AlphaMaskSampler, ( VertexStage.uvCoord + g_Layers[1].LayerOffset ) * g_Layers[1].LayerScale, 1.0f );
        
        // Blend layers
        float blendMask1 = ReadInput1D( g_Layers[1].BlendMask, g_TexBlendMask1, g_AlphaMaskSampler, ( VertexStage.uvCoord + g_Layers[1].LayerOffset ) * g_Layers[1].LayerScale, 1.0f );
        BlendDepthLayers( blendMask1, alphaMask, alphaCutoff, alphaMask1, alphaCutoff1 );
        
        if ( g_LayerCount > 2 ) {
            float alphaCutoff2 = g_Layers[2].AlphaCutoff;
            float alphaMask2 = ReadInput1D(g_Layers[2].AlphaMask, g_TexAlphaMask2, g_AlphaMaskSampler, ( VertexStage.uvCoord + g_Layers[2].LayerOffset ) * g_Layers[2].LayerScale,  1.0f );
            
            // Blend layers
            float blendMask2 = ReadInput1D( g_Layers[2].BlendMask, g_TexBlendMask2, g_AlphaMaskSampler, ( VertexStage.uvCoord + g_Layers[2].LayerOffset ) * g_Layers[2].LayerScale, 1.0f );
            BlendDepthLayers( blendMask2, alphaMask, alphaCutoff, alphaMask2, alphaCutoff2 );
        }
    }
#endif
#else
	FLAN_BUILD_DEPTH_LAYERS
#endif

#if PA_ENABLE_ALPHA_TEST
    if ( alphaMask < alphaCutoff ) {
        discard;
    }
#endif
}
