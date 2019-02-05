#include <CameraData.hlsli>
#include <LightsData.hlsli>
#include <BRDF.hlsli>

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

#if NYA_EDITOR
#include <MaterialShared.h>
#endif

#if NYA_BRDF_STANDARD
#include "ShadingModels/Standard.hlsl"
#else
#include "ShadingModels/Debug.hlsl"
#endif    

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
    output.position         = mul( float4( output.positionWS.xyz, 1.0f ), g_ViewProjectionMatrix );    
    output.previousPosition = mul( float4( output.positionWS.xyz, 1.0f ), g_PreviousViewProjectionMatrix );
   
    float4 PositionVS = mul( float4( output.positionWS.xyz, 1.0f ), g_ViewMatrix );
    output.depth = ( PositionVS.z / PositionVS.w );
    output.normal = normalize( mul( ModelMatrix, float4( VertexBuffer.Normal, 0.0f ) ) ).xyz;
    output.uvCoord = uvCoordinates;
    
	return output;
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

struct PixelStageData
{
    float4  Buffer0         : SV_TARGET0; // Shaded Surfaces Color
    float2  Buffer1         : SV_TARGET1; // Velocity (Opaque RenderList ONLY)
    float4  Buffer2         : SV_TARGET2; // Thin GBuffer: R: Subsurface Scattering Strength / GBA: Unused
};

Texture3D<uint> g_Clusters : register( t0 );
cbuffer ClusterBuffer : register( b1 )
{
    float3   g_ClustersScale;
    float3   g_ClustersBias;
};

float3 GetPointLightIlluminance( in PointLight light, in LightSurfaceInfos surface, in float depth, inout float3 L )
{
    float3 unormalizedL = light.PositionAndRadius.xyz - surface.PositionWorldSpace;
    float distance = length( unormalizedL );
    L = normalize( unormalizedL );

    float illuminance = pow( saturate( 1 - pow( ( distance / light.PositionAndRadius.w ), 4 ) ), 2 ) / ( distance * distance + 1 );
    float luminancePower = light.ColorAndPowerInLux.a / ( 4.0f * sqrt( light.PositionAndRadius.w * PI ) );

    return ( light.ColorAndPowerInLux.rgb ) * luminancePower * illuminance;
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
#endif
    
    float3 baseColor = float3( 1, 0, 0 );
    float metalness = 0.0f;
    float reflectance = 1.0f;
    float roughness = 1.0f;
    float ao = 1.0f;
    
    const LightSurfaceInfos surface = {
        V,
        ao,

        N,
        roughness,

        // Albedo
        lerp( baseColor, 0.0f, metalness ),
        metalness,

        // F0
        lerp( ( 0.16f * ( reflectance * reflectance ) ), baseColor, metalness ),

        // F90
        saturate( 50.0 * dot( reflectance, 0.33 ) ),

        VertexStage.positionWS.xyz,

        // LinearRoughness
        max( 0.01f, ( roughness * roughness ) ),

        // Reflection Vector
        R,

        // Disney BRDF LinearRoughness
        ( 0.5f + roughness / 2.0f ),

        // Clear Coat Normal
        N,

        // N dot V
        // FIX Use clamp instead of saturate (clamp dot product between epsilon and 1)
        clamp( dot( N, V ), 1e-5f, 1.0f ),

        0.0f,
        0.0f,

        0.0f,
        0u // Explicit Padding
    };

    // Outputs
    float4 LightContribution = float4( 0, 0, 0, 1 );
    float2 Velocity = float2( 0, 0 );
 
    // Compute cluster and fetch its light mask
	int4 coord = int4( VertexStage.positionWS.xyz * g_ClustersScale + g_ClustersBias, 0 );
	uint light_mask = g_Clusters.Load( coord );
    
	while ( light_mask ) {
		// Extract a light from the mask and disable that bit
		uint i = firstbitlow( light_mask );
        
        // Do lighting
        float3 L;
		PointLight light = PointLights[i];
        float3 pointLightIlluminance = GetPointLightIlluminance( light, surface, VertexStage.depth, L );        
		LightContribution.rgb += DoShading( L, surface ) * pointLightIlluminance;
		
		light_mask &= ~( 1 << i );
    }
    
    float2 prevPositionSS = (VertexStage.previousPosition.xy / VertexStage.previousPosition.z) * float2(0.5f, -0.5f) + 0.5f;
    //prevPositionSS *= float2( g_BackbufferDimension );
   
    Velocity = VertexStage.position.xy - prevPositionSS;
    Velocity -= g_CameraJitteringOffset;
    
    // Write output to buffer(s)
    PixelStageData output;
    output.Buffer0 = LightContribution;
    output.Buffer1 = Velocity;
    output.Buffer2 = float4( 0, 0, 0, 0 );

    return output;
}
