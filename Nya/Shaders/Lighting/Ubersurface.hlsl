#include <CameraData.hlsli>
#include <LightsData.hlsli>

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
    
    float3 Tangent, Binormal;
    
#if NYA_USE_NORMAL_MAPPING
    float3x3 TBNMatrix = ComputeTangentFrame( N, VertexStage.positionWS.xyz, VertexStage.uvCoord, Tangent, Binormal );
#endif
    
    // Outputs
    float4 LightContribution = float4( 0, 0, 0, 1 );
    float2 Velocity = float2( 0, 0 );
 
    // Compute cluster and fetch its light mask
	int4 coord = int4( VertexStage.positionWS.xyz * g_ClustersScale + g_ClustersBias, 0 );
	uint light_mask = g_Clusters.Load( coord );
    
	while ( light_mask ) {
		// Extract a light from the mask and disable that bit
		uint i = firstbitlow( light_mask );
        
		PointLight light = PointLights[i];
		LightContribution.rgb += light.ColorAndPowerInLux.rgb;
		
        // Do lighting
        
		light_mask &= ~( 1 << i );
    }
    
    // Write output to buffer(s)
    PixelStageData output;
    output.Buffer0 = LightContribution;
    output.Buffer1 = Velocity;
    output.Buffer2 = float4( 0, 0, 0, 0 );

    return output;
}
