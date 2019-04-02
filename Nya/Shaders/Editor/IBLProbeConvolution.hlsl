#include <MathsHelpers.hlsli>
#include <PhotometricHelpers.hlsli>

TextureCubeArray g_IBLProbeArray : register( t20 );

sampler g_BilinearSampler : register( s0 );

static const uint sampleCount = 2048;

struct VertexStageOutput
{
	float4	Position			: SV_POSITION;			// The vertex position to rasterize.
	float3	Direction			: TEXCOORD0;			// The matching normal map sampling direction for the vertex position (the viewport quad maps to a cubemap face).
	float2	UV					: TEXCOORD1;
};

struct PixelStageData
{
    float4	Diffuse			: SV_TARGET0;
    float4	Specular    	: SV_TARGET1;
};

cbuffer ConvolutionData : register( b0 )
{
	float3 dirX;
	float roughness;

	float3 dirY;
	float width;
    
	float3 dirZ;
    uint probeIndex;
};

float radicalInverse_VdC( uint bits )
{
    return reversebits( bits ) * 2.3283064365386963e-10f; // / 0x100000000
}

float2 getSample( uint i, uint N )
{
    return float2( float( i ) / float( N ), radicalInverse_VdC( i ) );
}

void importanceSampleCosDir(
    in float2 u,
    in float3 N,
    out float3 L,
    out float NdotL,
    out float pdf )
{
    // Local referencial
    float3 upVector = abs( N.z ) < 0.999 ? float3( 0, 0, 1 ) : float3( 1, 0, 0 );
    float3 tangentX = normalize( cross( upVector, N ) );
    float3 tangentY = cross( N, tangentX );

    float u1 = u.x;
    float u2 = u.y;

    float r = sqrt( u1 );
    float phi = u2 * PI * 2.0f;

    L = float3( r*cos( phi ), r*sin( phi ), sqrt( max( 0.0f, 1.0f - u1 ) ) );
    L = normalize( tangentX * L.y + tangentY * L.x + N * L.z );

    NdotL = dot( L, N );
    pdf = NdotL * INV_PI;
}

float4 integrateDiffuseCube( in float3 N )
{
    float3 accBrdf = float3( 0, 0, 0 );

    for ( uint i = 0; i < sampleCount; ++i ) {
        float2 eta = getSample( i, sampleCount );

        float3 L;
        float NdotL;
        float pdf;

        // see reference code in appendix
        importanceSampleCosDir( eta, N, L, NdotL, pdf );

        if ( NdotL > 0 ) {
            accBrdf += DecodeRGBD( g_IBLProbeArray.SampleLevel( g_BilinearSampler, float4( L, probeIndex ), 0 ) );
        }
    }

    return float4 ( accBrdf * ( 1.0f / sampleCount ), 1.0f );
}

struct Referential
{
    float3 upVector;
    float3 tangentX;
    float3 tangentY;
};

Referential CreateReferential( in float3 N )
{
    Referential referential;
    referential.upVector = abs( N.z ) < 0.999 ? float3( 0, 0, 1 ) : float3( 1, 0, 0 );
    referential.tangentX = normalize( cross( referential.upVector, N ) );
    referential.tangentY = cross( N, referential.tangentX );
    return referential;
}

float D_GGX( float NdotH, float m )
{
    // Divide by PI is apply later
    float m2 = m * m;
    float f = ( NdotH * m2 - NdotH ) * NdotH + 1;
    return m2 / ( f * f );
}

float D_GGX_Divide_Pi( float NdotH, float roughness )
{
    return D_GGX( NdotH, roughness ) / PI;
}

float3 ImportanceSampleGGX( float2 Xi, float Roughness, float3 N, Referential referential )
{
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt( ( 1 - Xi.y ) / ( 1 + ( a*a - 1 ) * Xi.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );
    float3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;
    // Tangent to world space
    return referential.tangentX * H.x + referential.tangentY * H.y + N * H.z;
}

void ImportanceSampleGGXDir( in float2 Xi, in float3 V, in float3 N, in float Roughness, out float3 H, out float3 L )
{
    Referential referential = CreateReferential( N );
    H = ImportanceSampleGGX( Xi, Roughness, N, referential );
    L = 2 * dot( V, H ) * H - V;
}

float3 IntegrateCubeLDOnly( in float3 V, in float3 N, in float roughness )
{
    float3 accBrdf = float3( 0, 0, 0 );
    float accBrdfWeight = 0;
    for ( uint i = 0; i<sampleCount; ++i ) {
        float2 eta = getSample( i, sampleCount );
        float3 L;
        float3 H;
        ImportanceSampleGGXDir( eta, V, N, roughness, H, L );
        float NdotL = dot( N, L );
        if ( NdotL > 0 ) {
            // Use pre-filtered importance sampling (i.e use lower mipmap
            // level for fetching sample with low probability in order
            // to reduce the variance).
            // (Reference: GPU Gem3)
            //
            // Since we pre-integrate the result for normal direction ,
            // N == V and then NdotH == LdotH. This is why the BRDF pdf
            // can be simplifed from:
            // pdf = D_GGX_Divide_Pi(NdotH , roughness)*NdotH/(4*LdotH);
            // to
            // pdf = D_GGX_Divide_Pi(NdotH , roughness) / 4;
            //
            // The mipmap level is clamped to something lower than 8x8
            // in order to avoid cubemap filtering issues
            //
            // - OmegaS: Solid angle associated to a sample
            // - OmegaP: Solid angle associated to a pixel of the cubemap
#define mipCount 8
            float mipLevel = 0;
            float NdotH = saturate( dot( N, H ) );
            float LdotH = saturate( dot( L, H ) );
            float pdf = D_GGX_Divide_Pi( NdotH, roughness ) * NdotH / ( 4 * LdotH );
            if ( pdf > 0.0 ) {
                float omegaS = 1.0 / ( sampleCount * pdf );
                float omegaP = 4.0 * PI / ( 6.0 * width * width );
                float mipLevel = clamp( 0.5 * log2( omegaS / omegaP ), 0, mipCount );

                mipLevel = clamp( 0.5 * log2( omegaS / omegaP ), 0, mipCount );
            }

            float3 Li = DecodeRGBD( g_IBLProbeArray.SampleLevel( g_BilinearSampler, float4( L, probeIndex ), mipLevel ) );

            accBrdf += Li * NdotL;
            accBrdfWeight += NdotL;
        }
    }

    return accBrdf * ( 1.0f / accBrdfWeight );
}

VertexStageOutput EntryPointVS( uint id : SV_VERTEXID )
{
    VertexStageOutput OUT = (VertexStageOutput)0;

    float2 tex = float2( id & 1, id >> 1 );
    OUT.UV = tex;

    float4 pos = float4( tex * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f ), 1.0f, 1.0f );
    OUT.Position = float4( ( tex.x - 0.5f ) * 2.0f, -( tex.y - 0.5f ) * 2.0f, 0.0f, 1.0f );

    OUT.Direction = dirX * pos.x
        + dirY * pos.y
        + dirZ * pos.z;

    return OUT;
}

PixelStageData EntryPointPS( VertexStageOutput p )
{
    PixelStageData output = (PixelStageData)0;

    float3 dir = normalize( p.Direction );
    output.Diffuse = integrateDiffuseCube( dir );
    output.Specular = float4( IntegrateCubeLDOnly( dir, dir, roughness ), 1.0f );

    return output;
}
