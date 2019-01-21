#include <CameraData.hlsli>

struct DEFAULT_VS_OUT 
{ 
    float4 position : SV_Position; 
    float4 viewRay : POSITION0;
};

DEFAULT_VS_OUT EntryPointVS( uint id : SV_VERTEXID )
{
    DEFAULT_VS_OUT output;
    
    float x = ( float )( id / 2 );
    float y = ( float )( id % 2 );

    // Clip-space position
    output.position = float4(
        x * 4.0 - 1.0,
        y * 4.0 - 1.0,
        0.0,
        1.0
    );

    output.viewRay = mul( output.position, g_InverseViewProjectionMatrix );

    return output;
}


cbuffer HosekSkyModel : register( b1 )
{
	float3 A;
	float3 B;
	float3 C;
	float3 D;
	float3 E;
	float3 F;
	float3 G;
	float3 H;
	float3 I;
	float3 Z;
	
	float3 g_SunDirection;
}

// ArHosekSkyModel_GetRadianceInternal
float3 HosekWilkie(float cos_theta, float gamma, float cos_gamma)
{
	float3 chi = ( 1 + cos_gamma * cos_gamma ) / pow( 1 + H * H - 2 * cos_gamma * H, float3( 1.5, 1.5, 1.5 ) );
    return (1 + A * exp(B / (cos_theta + 0.01))) * (C + D * exp(E * gamma) + F * (cos_gamma * cos_gamma) + G * chi + I * sqrt(cos_theta));
}

float4 EntryPointPS( DEFAULT_VS_OUT VertexStage ) : SV_TARGET0
{
	float3 V = normalize( VertexStage.viewRay ).xyz;
	
	float cos_theta = saturate( V.y );
	float cos_gamma = saturate( dot( V, g_SunDirection ) );
	float gamma_ = acos( cos_gamma );

	float3 R = Z * HosekWilkie( cos_theta, gamma_, cos_gamma );
	
    return float4( R, 1.0f );
}
