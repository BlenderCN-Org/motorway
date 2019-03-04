#include <CameraData.hlsli>
#include <LightsData.hlsli>
#include <BRDF.hlsli>
#include <ShadowMappingShared.h>

Buffer<float4> g_InstanceVectorBuffer : register( t8 );
cbuffer InstanceBuffer : register( b1 )
{
    float   g_StartVector;
    float   g_VectorPerInstance;
};

struct VertexBufferData
{
	float3 Position         : POSITION0;
	float3 Normal           : NORMAL0;
	float2 TexCoordinates   : TEXCOORD0;
};

struct VertexStageData
{
    float4 position		: SV_POSITION;
    float4 positionWS   : POSITION0;
    float4 previousPosition : POSITION1;
    float3 normal       : NORMAL0;
    float depth         : DEPTH;
    float2 uvCoord      : TEXCOORD0;
};

float4x4 GetInstanceModelMatrix( const uint instanceIdx )
{
    uint modelMatrixVectorOffset = g_StartVector + instanceIdx * g_VectorPerInstance + 0;
    
    float4 r0 = g_InstanceVectorBuffer.Load( modelMatrixVectorOffset + 0 );
    float4 r1 = g_InstanceVectorBuffer.Load( modelMatrixVectorOffset + 1 );
    float4 r2 = g_InstanceVectorBuffer.Load( modelMatrixVectorOffset + 2 );
    float4 r3 = g_InstanceVectorBuffer.Load( modelMatrixVectorOffset + 3 );
    
    return float4x4( r0, r1, r2, r3 );
}

VertexStageData EntryPointVS( VertexBufferData VertexBuffer, uint InstanceId : SV_InstanceID )
{
    VertexStageData output = (VertexStageData)0;
    
    float4x4 ModelMatrix = GetInstanceModelMatrix( InstanceId );

    float2 uvCoordinates =  VertexBuffer.TexCoordinates;
    
#if NYA_SCALE_UV_BY_MODEL_SCALE
    float scaleX = length( float3( ModelMatrix._11, ModelMatrix._12, ModelMatrix._13 ) );
    float scaleY = length( float3( ModelMatrix._21, ModelMatrix._22, ModelMatrix._23 ) );

    uvCoordinates *= float2( scaleX, scaleY );
#endif

    output.positionWS       = mul( ModelMatrix, float4( VertexBuffer.Position, 1.0f ) );
    output.position         = mul( g_ViewProjectionMatrix, float4( output.positionWS.xyz, 1.0f ) );    
    output.previousPosition = mul( g_PreviousViewProjectionMatrix, float4( output.positionWS.xyz, 1.0f ) );
   
    float4 PositionVS = mul( g_ViewMatrix, float4( output.positionWS.xyz, 1.0f ) );
    output.depth = ( PositionVS.z / PositionVS.w );
    output.normal = normalize( mul( ModelMatrix, float4( VertexBuffer.Normal, 0.0f ) ) ).xyz;
    output.uvCoord = uvCoordinates;
    
	return output;
}

float3 accurateSRGBToLinear( in float3 sRGBCol )
{
    float3 linearRGBLo = sRGBCol / 12.92;

    // Ignore X3571, incoming vector will always be superior to 0
    float3 linearRGBHi = pow( abs( ( sRGBCol + 0.055 ) / 1.055 ), 2.4 );

    float3 linearRGB = ( sRGBCol <= 0.04045 ) ? linearRGBLo : linearRGBHi;

    return linearRGB;
}

float3x3 ComputeTangentFrame( const float3 N, const float3 P, const float2 UV, out float3 T, out float3 B )
{
    // ddx_coarse can be faster than ddx, but could result in artifacts. Haven't observed any artifacts yet.
    float3 dp1 = ddx_coarse( P );
    float3 dp2 = ddy_coarse( P );
    float2 duv1 = ddx_coarse( UV );
    float2 duv2 = ddy_coarse( UV );

    float3x3 M = float3x3( dp1, dp2, cross( dp1, dp2 ) );
    float2x3 inverseM = float2x3( cross( M[1], M[2] ), cross( M[2], M[0] ) );
    T = normalize( mul( float2( duv1.x, duv2.x ), inverseM ) );
    B = normalize( mul( float2( duv1.y, duv2.y ), inverseM ) );

    return float3x3( T, B, N );
}

// PixelShader
#include <SceneInfos.hlsli>

struct PixelStageData
{
    float4  Buffer0         : SV_TARGET0; // Shaded Surfaces Color
    float2  Buffer1         : SV_TARGET1; // Velocity (Opaque RenderList ONLY)
    float3  Buffer2         : SV_TARGET2; // Thin GBuffer: RG: Normal (Spheremap encoded) / B: Roughness
};

Texture3D<uint> g_Clusters : register( t0 );

struct LightSurfaceInfos
{
    float3  V;
    float   AmbientOcclusion;

    float3  N;
    float   Roughness;

    float3  Albedo;
    float   Metalness;

    float3  FresnelColor;
    float   F90;

    float3  PositionWorldSpace;
    float   LinearRoughness;

    float3  R;
    float   DisneyRoughness;

    float3  SecondaryNormal;
    float   NoV;

    float   ClearCoat;
    float   ClearCoatGlossiness;
    float   SubsurfaceScatteringStrength; // 0..1 range
    uint    PADDING;
};

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

#if NYA_EDITOR
#include <MaterialShared.h>

sampler					g_BRDFInputsSampler : register( s1 );

// t0 => Light clusters
// t1 => Shading Model BRDF DFG LUT
// t2 => Sun CSM ShadowMap

Texture2D g_TexBaseColor0 : register( t3 );
Texture2D g_TexAlphaMask0 : register( t4 );
Texture2D g_TexReflectance0 : register( t5 );
Texture2D g_TexRoughness0 : register( t6 );
Texture2D g_TexMetalness0 : register( t7 );
Texture2D g_TexAmbientOcclusion0 : register( t8 );
Texture2D g_TexNormal0 : register( t9 );
Texture2D g_TexDisplacement0 : register( t10 );
Texture2D g_TexEmissivity0 : register( t11 );
Texture2D g_TexClearCoatNormal0 : register( t12 );
Texture2D g_TexBlendMask0 : register( t13 );

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
    layer.BaseColor = ReadInput3D( g_Layers[layerIdx].BaseColor, g_TexBaseColor##layerIdx, g_BRDFInputsSampler, uvCoords, float3( 0.42, 0.42, 0.42 ) );\
	if ( g_Layers[layerIdx].BaseColor.SamplingMode == SAMPLING_MODE_SRGB ) {\
        layer.BaseColor = accurateSRGBToLinear( layer.BaseColor );\
    }\
	layer.Roughness = ReadInput1D( g_Layers[layerIdx].Roughness, g_TexRoughness##layerIdx, g_BRDFInputsSampler, uvCoords, 1.0f );\
    if ( g_Layers[layerIdx].Roughness.SamplingMode == SAMPLING_MODE_ALPHA_ROUGHNESS ) {\
        layer.Roughness = ( layer.Roughness * layer.Roughness );\
    }\
	layer.Metalness = ReadInput1D( g_Layers[layerIdx].Metalness, g_TexMetalness##layerIdx, g_BRDFInputsSampler, uvCoords, 0.0f );\
    layer.AmbientOcclusion = ReadInput1D( g_Layers[layerIdx].AmbientOcclusion, g_TexAmbientOcclusion##layerIdx, g_BRDFInputsSampler, uvCoords, 1.0f );\
	\
	layer.Emissivity = ReadInput1D( g_Layers[layerIdx].Emissivity, g_TexEmissivity##layerIdx, g_BRDFInputsSampler, uvCoords, 0.0f );\
	layer.Reflectance = ReadInput1D( g_Layers[layerIdx].Reflectance, g_TexReflectance##layerIdx, g_BRDFInputsSampler, uvCoords, 1.0f );\
	needNormalMapUnpack = ( g_Layers[layerIdx].Normal.Type == INPUT_TYPE_TEXTURE && g_Layers[layerIdx].Normal.SamplingMode == SAMPLING_MODE_TANGENT_SPACE );\
    if ( needNormalMapUnpack ) {\
        float4 sampledTexture = g_TexNormal##layerIdx.Sample( g_BRDFInputsSampler, uvCoords );\
        layer.Normal = normalize( ( sampledTexture.rgb * g_Layers[layerIdx].NormalMapStrength ) * 2.0f - 1.0f );\
    } else {\
        layer.Normal = N;\
    }\
	layer.SecondaryNormal = float3( 0, 1, 0 );\
    layer.AlphaMask = 1.0f;\
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
	layer.BlendMask = ReadInput1D( g_Layers[layerIdx].BlendMask, g_TexBlendMask##layerIdx, g_BRDFInputsSampler, uvCoords, 1.0f );\
    \
	return layer;\
}\

NYA_READ_LAYER( 0 )
#endif

#if NYA_BRDF_STANDARD
#include "ShadingModels/Standard.hlsl"
#else
#include "ShadingModels/Debug.hlsl"
#endif    

float2 EncodeNormals( const in float3 n )
{
    float2 enc = normalize(n.xy) * (sqrt(-n.z*0.5+0.5));
    enc = enc*0.5+0.5;
    
    return enc;
}

float2 ComputeReceiverPlaneDepthBias( float3 texCoordDX, float3 texCoordDY )
{
    float2 biasUV;
    biasUV.x = texCoordDY.y * texCoordDX.z - texCoordDX.y * texCoordDY.z;
    biasUV.y = texCoordDX.x * texCoordDY.z - texCoordDY.x * texCoordDX.z;
    biasUV *= 1.0f / ( ( texCoordDX.x * texCoordDY.y ) - ( texCoordDX.y * texCoordDY.x ) );

    return biasUV;
}

SamplerComparisonState 	g_ShadowMapSampler : register( s2 );
Texture2D g_SunShadowMap : register( t1 );

float SampleCascadedShadowMap( in float2 base_uv, in float u, in float v, in float2 shadowMapSizeInv, in uint cascadeIdx, in float depth, in float2 receiverPlaneDepthBias )
{
    float2 uv = base_uv + float2( u, v ) * shadowMapSizeInv;

    float z = depth + dot( float2( u, v ) * shadowMapSizeInv, receiverPlaneDepthBias );

    float2 shiftedCoordinates = uv.xy;
    shiftedCoordinates.x = float( cascadeIdx ) / float(CSM_SLICE_COUNT);
    shiftedCoordinates.x += ( uv.x / float(CSM_SLICE_COUNT) );
    
    return g_SunShadowMap.SampleCmpLevelZero( g_ShadowMapSampler, shiftedCoordinates.xy, z );
}

float SampleCascadedShadowMapOptimizedPCF( in float3 shadowPos, in float3 shadowPosDX, in float3 shadowPosDY, in uint cascadeIdx )
{
    static const float2 shadowMapSize = float2( CSM_SHADOW_MAP_DIMENSIONS, CSM_SHADOW_MAP_DIMENSIONS );

    float lightDepth = shadowPos.z;
    float2 texelSize = 1.0f / shadowMapSize;

    float2 receiverPlaneDepthBias = ComputeReceiverPlaneDepthBias( shadowPosDX, shadowPosDY );

    // Static depth biasing to make up for incorrect fractional sampling on the shadow map grid
    float fractionalSamplingError = 2 * dot( float2( 1.0f, 1.0f ) * texelSize, abs( receiverPlaneDepthBias ) );
    lightDepth -= min( fractionalSamplingError, 0.01f );

    float2 uv = shadowPos.xy * shadowMapSize; // 1 unit - 1 texel
    float2 shadowMapSizeInv = 1.0 / shadowMapSize;

    float2 base_uv;
    base_uv.x = floor( uv.x + 0.5 );
    base_uv.y = floor( uv.y + 0.5 );

    float s = ( uv.x + 0.5 - base_uv.x );
    float t = ( uv.y + 0.5 - base_uv.y );

    base_uv -= float2( 0.5, 0.5 );
    base_uv *= shadowMapSizeInv;

    float sum = 0;
    float uw0 = ( 5 * s - 6 );
    float uw1 = ( 11 * s - 28 );
    float uw2 = -( 11 * s + 17 );
    float uw3 = -( 5 * s + 1 );

    float u0 = ( 4 * s - 5 ) / uw0 - 3;
    float u1 = ( 4 * s - 16 ) / uw1 - 1;
    float u2 = -( 7 * s + 5 ) / uw2 + 1;
    float u3 = -s / uw3 + 3;

    float vw0 = ( 5 * t - 6 );
    float vw1 = ( 11 * t - 28 );
    float vw2 = -( 11 * t + 17 );
    float vw3 = -( 5 * t + 1 );

    float v0 = ( 4 * t - 5 ) / vw0 - 3;
    float v1 = ( 4 * t - 16 ) / vw1 - 1;
    float v2 = -( 7 * t + 5 ) / vw2 + 1;
    float v3 = -t / vw3 + 3;

    sum += uw0 * vw0 * SampleCascadedShadowMap( base_uv, u0, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw1 * vw0 * SampleCascadedShadowMap( base_uv, u1, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw2 * vw0 * SampleCascadedShadowMap( base_uv, u2, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw3 * vw0 * SampleCascadedShadowMap( base_uv, u3, v0, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );

    sum += uw0 * vw1 * SampleCascadedShadowMap( base_uv, u0, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw1 * vw1 * SampleCascadedShadowMap( base_uv, u1, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw2 * vw1 * SampleCascadedShadowMap( base_uv, u2, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw3 * vw1 * SampleCascadedShadowMap( base_uv, u3, v1, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );

    sum += uw0 * vw2 * SampleCascadedShadowMap( base_uv, u0, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw1 * vw2 * SampleCascadedShadowMap( base_uv, u1, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw2 * vw2 * SampleCascadedShadowMap( base_uv, u2, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw3 * vw2 * SampleCascadedShadowMap( base_uv, u3, v2, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );

    sum += uw0 * vw3 * SampleCascadedShadowMap( base_uv, u0, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw1 * vw3 * SampleCascadedShadowMap( base_uv, u1, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw2 * vw3 * SampleCascadedShadowMap( base_uv, u2, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );
    sum += uw3 * vw3 * SampleCascadedShadowMap( base_uv, u3, v3, shadowMapSizeInv, cascadeIdx, lightDepth, receiverPlaneDepthBias );

    return sum * 1.0f / 2704;
}

float3 GetCascadedShadowMapShadowPosOffset( in float nDotL, in float3 normal )
{
    static const float texelSize = 2.0f / CSM_SHADOW_MAP_DIMENSIONS;
    float nmlOffsetScale = saturate( 1.0f - nDotL );
    return texelSize * 1.0f * nmlOffsetScale * normal;
}

float3 SampleShadowCascade( in DirectionalLight light, in float3 shadowPosition, in float3 shadowPosDX, in float3 shadowPosDY, in uint cascadeIdx )
{
    shadowPosition += g_CascadeOffsets[cascadeIdx].xyz;
    shadowPosition *= g_CascadeScales[cascadeIdx].xyz;

    shadowPosDX *= g_CascadeScales[cascadeIdx].xyz;
    shadowPosDY *= g_CascadeScales[cascadeIdx].xyz;

    float3 cascadeColor = 1.0f;
	
#if NYA_DEBUG_CSM_CASCADE
	const float3 CascadeColors[4] =
	{
		float3( 1.0f, 0.0, 0.0f ),
		float3( 0.0f, 1.0f, 0.0f ),
		float3( 0.0f, 0.0f, 1.0f ),
		float3( 1.0f, 1.0f, 0.0f )
	};

	cascadeColor = CascadeColors[cascadeIdx];
#endif

    return cascadeColor + SampleCascadedShadowMapOptimizedPCF( shadowPosition, shadowPosDX, shadowPosDY, cascadeIdx );
}

float3 GetPointLightIlluminance( in PointLight light, in LightSurfaceInfos surface, in float depth, inout float3 L )
{
    float3 unormalizedL = light.PositionAndRadius.xyz - surface.PositionWorldSpace;
    float distance = length( unormalizedL );
    L = normalize( unormalizedL );

    float illuminance = pow( saturate( 1 - pow( ( distance / light.PositionAndRadius.w ), 4 ) ), 2 ) / ( distance * distance + 1 );
    float luminancePower = light.ColorAndPowerInLux.a / ( 4.0f * sqrt( light.PositionAndRadius.w * PI ) );

    return ( light.ColorAndPowerInLux.rgb ) * luminancePower * illuminance;
}

float3 GetDirectionalLightIlluminance( in DirectionalLight light, in LightSurfaceInfos surface, in float depth, inout float3 L )
{
    float r = sin( light.SunColorAndAngularRadius.w );
    float d = cos( light.SunColorAndAngularRadius.w );

    float DoR = dot( light.SunDirectionAndIlluminanceInLux.xyz, surface.R );
    float3 S = surface.R - DoR * light.SunDirectionAndIlluminanceInLux.xyz;

    L = ( DoR < d ) ? normalize( d * light.SunDirectionAndIlluminanceInLux.xyz + normalize( S ) * r ) : surface.R;

    float illuminance = light.SunDirectionAndIlluminanceInLux.w * saturate( dot( surface.N, light.SunDirectionAndIlluminanceInLux.xyz ) );

    // Add shadow
    // We assume the surface is lit if not covered by the shadow map
    float3 shadowVisibility = 1.0f;
    float3 surfaceTransmittance = float3( 0, 0, 0 );
    
    // Figure out which cascade to sample from
    uint cascadeIdx = ~0;

    if ( depth <= g_ShadowSplitDistances.x ) cascadeIdx = 0;
    else if ( depth <= g_ShadowSplitDistances.y ) cascadeIdx = 1;
    else if ( depth <= g_ShadowSplitDistances.z ) cascadeIdx = 2;
    else if ( depth <= g_ShadowSplitDistances.w ) cascadeIdx = 3;

    [branch]
    if ( cascadeIdx <= 3 ) {
        float NoL = saturate( dot( surface.N, L ) );

        // Apply offset
        float3 offset = GetCascadedShadowMapShadowPosOffset( NoL, surface.N ) / abs( g_CascadeScales[cascadeIdx].z );

        // Project into shadow space
        float3 samplePos = surface.PositionWorldSpace + offset;
        float3 shadowPosition = mul( float4( samplePos, 1.0f ), g_ShadowMatrixShared ).xyz;
        float3 shadowPosDX = ddx_fine( shadowPosition );
        float3 shadowPosDY = ddy_fine( shadowPosition );

        shadowVisibility = SampleShadowCascade( light, shadowPosition, shadowPosDX, shadowPosDY, cascadeIdx );

        // Make sure the value is within a valid range (avoid NaN)
        shadowVisibility = saturate( shadowVisibility );

        // Sample the next cascade, and blend between the two results to
        // smooth the transition
        float nextSplit = g_ShadowSplitDistances[cascadeIdx];
        float splitSize = ( cascadeIdx == 0 ) ? nextSplit : nextSplit - g_ShadowSplitDistances[cascadeIdx - 1];
        float fadeFactor = ( nextSplit - depth ) / splitSize;

        [branch]
        if ( fadeFactor <= CSM_SLICE_BLEND_THRESHOLD && cascadeIdx != CSM_SLICE_COUNT - 1 ) {
            float3 nextSplitVisibility = SampleShadowCascade( light, shadowPosition, shadowPosDX, shadowPosDY, cascadeIdx + 1 );
            float lerpAmt = smoothstep( 0.0f, CSM_SLICE_BLEND_THRESHOLD, fadeFactor );
            shadowVisibility = lerp( nextSplitVisibility, shadowVisibility, lerpAmt );
            shadowVisibility = saturate( shadowVisibility );
        }
    }

    // [branch]
    // if ( surface.SubsurfaceScatteringStrength > 0.0f ) { 
        // float NoL = saturate( dot( surface.N, L ) );

        // // Apply offset
        // float3 offset = GetCascadedShadowMapShadowPosOffset( NoL, surface.N ) / abs( CascadeScales[cascadeIdx].z );

        // // Project into shadow space
        // float3 samplePos = surface.PositionWorldSpace + offset;
        // float4 shrinkedPos = float4(surface.PositionWorldSpace - 0.005 * surface.N, 1.0);
        // float3 ssShadowPosition = mul( shrinkedPos, ShadowMatrixShared ).xyz;
        // float3 ssShadowPosDX = ddx_fine( ssShadowPosition );
        // float3 ssShadowPosDY = ddy_fine( ssShadowPosition );

        // float d1 = SampleShadowCascade( light, ssShadowPosition, ssShadowPosDX, ssShadowPosDY, cascadeIdx ).g; //SSSSSample(shadowMap, shadowPosition.xy / shadowPosition.w).r; // 'd1' has a range of 0..1
        // d1 = saturate( d1 );
        
        // surfaceTransmittance = SSSSTransmittance( surface.SubsurfaceScatteringStrength, 0.005f, surface.N, L, ( d1 * 128.0f ), ssShadowPosition.z );
    // }    

    // Get Sun Irradiance
    // NOTE Do not add sky irradiance since we already have IBL as ambient term
    float3 skyIrradiance = float3( 0, 0, 0 );
    float3 sunIrradiance = float3( 1, 1, 1 ); //GetSunAndSkyIrradiance( surface.PositionWorldSpace.xzy - g_EarthCenter, surface.N.xzy, g_SunDirection, skyIrradiance );
    float3 lightIlluminance = ( sunIrradiance * illuminance * shadowVisibility.rgb );
    
    return lightIlluminance + ( lightIlluminance * surfaceTransmittance );
}

PixelStageData EntryPointPS( VertexStageData VertexStage, bool isFrontFace : SV_IsFrontFace )
{
#if NYA_USE_LOD_ALPHA_BLENDING
    // [branch]
    // if ( g_LodBlendAlpha != 1.0f ) {
        // static const float4x4 thresholdMatrix = {
            // 1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
            // 13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
            // 4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
            // 16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
        // };

        // clip( g_LodBlendAlpha - thresholdMatrix[VertexStage.position.x % 4][VertexStage.position.y % 4] );
    // }
#endif

    // Compute common terms from vertex stage variables
    float3 N = normalize( VertexStage.normal );
    float3 V = normalize( g_WorldPosition.xyz - VertexStage.positionWS.xyz );
    float3 R = reflect( -V, N );
    
    float3 Tangent, Binormal;
    
#if NYA_USE_NORMAL_MAPPING
    float3x3 TBNMatrix = ComputeTangentFrame( N, VertexStage.positionWS.xyz, VertexStage.uvCoord, Tangent, Binormal );
#else
	static const float3x3 TBNMatrix = float3x3( 1, 0, 0, 0, 1, 0, 0, 0, 1 );
#endif
    
	bool needNormalMapUnpack = false, needSecondaryNormalMapUnpack = false;
	
#if NYA_EDITOR
	MaterialReadLayer layer = ReadLayer0( VertexStage, N, V, TBNMatrix, needNormalMapUnpack, needSecondaryNormalMapUnpack );
#else
	MaterialReadLayer layer;
	layer.BaseColor = float3( 0.42, 0.42, 0.42 );
	layer.Metalness = 0.0f;
	layer.Roughness = 1.0f;
	layer.Reflectance = 1.0f;
	layer.AmbientOcclusion = 1.0f;
	layer.Normal = N;
#endif

	if ( needNormalMapUnpack ) {
        layer.Normal = normalize( mul( layer.Normal, TBNMatrix ) );
    }
	
	// Build surface infos
    const LightSurfaceInfos surface = {
        V,
        layer.AmbientOcclusion,

        layer.Normal,
        layer.Roughness,

        // Albedo
        lerp( layer.BaseColor, 0.0f, layer.Metalness ),
        layer.Metalness,

        // F0
        lerp( ( 0.16f * ( layer.Reflectance * layer.Reflectance ) ), layer.BaseColor, layer.Metalness ),

        // F90
        saturate( 50.0 * dot( layer.Reflectance, 0.33 ) ),

        VertexStage.positionWS.xyz,

        // LinearRoughness
        max( 0.01f, ( layer.Reflectance * layer.Reflectance ) ),

        // Reflection Vector
        R,

        // Disney BRDF LinearRoughness
        ( 0.5f + layer.Reflectance / 2.0f ),

        // Clear Coat Normal
        layer.Normal,

        // N dot V
        // FIX Use clamp instead of saturate (clamp dot product between epsilon and 1)
        clamp( dot( layer.Normal, V ), 1e-5f, 1.0f ),

        0.0f,
        0.0f,

        0.0f,
        0u // Explicit Padding
    };

    // Outputs
    float4 LightContribution = float4( 0, 0, 0, 1 );
    float2 Velocity = float2( 0, 0 );
    
    // Add explicit sun/moon light contribution
    float3 L;
    float3 dirLightIlluminance = GetDirectionalLightIlluminance( g_DirectionalLight, surface, VertexStage.depth, L );        
    LightContribution.rgb += DoShading( L, surface ) * dirLightIlluminance;
    
    // Compute cluster and fetch its light mask
	int4 coord = int4( VertexStage.positionWS.xyz * g_ClustersScale + g_ClustersBias, 0 );
	uint light_mask = g_Clusters.Load( coord );
    
	while ( light_mask ) {
		// Extract a light from the mask and disable that bit
		uint i = firstbitlow( light_mask );
		light_mask &= ~( 1 << i );
        
        // Do lighting
        float3 L;
		PointLight light = g_PointLights[i];
        float3 pointLightIlluminance = GetPointLightIlluminance( light, surface, VertexStage.depth, L );        
		LightContribution.rgb += DoShading( L, surface ) * pointLightIlluminance;	
    }
    
    float2 screenSize = g_ScreenSize * g_ImageQuality;
    
    float2 prevPositionSS = ( VertexStage.previousPosition.xy / VertexStage.previousPosition.w ) * float2( 0.5f, -0.5f ) + 0.5f;
    prevPositionSS *= screenSize;
   
    Velocity = ( VertexStage.position.xy - prevPositionSS );
    Velocity -= g_CameraJitteringOffset;
    Velocity /= screenSize;
    
    // Write output to buffer(s)
    const float2 EncodedNormals = EncodeNormals( surface.N );
    
    PixelStageData output;
    output.Buffer0 = LightContribution;
    output.Buffer1 = Velocity;
    output.Buffer2 = float3( EncodedNormals, surface.Roughness );

    return output;
}
