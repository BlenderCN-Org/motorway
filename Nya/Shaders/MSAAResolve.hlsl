#include <PhotometricHelpers.hlsli>
#include <AutoExposure/SharedAutoExposure.hlsli>

struct VertexStageData
{
	float4	Position : SV_POSITION;
	float2	TexCoordinates : TEXCOORD;
};

Texture2DMS<float4> g_InputTexture 			: register( t0 );
Texture2DMS<float2> g_VelocityTexture 		: register( t1 );
Texture2DMS<float> g_DepthTexture 			: register( t2 );
Texture2D<float4> g_LastFrameInputTexture 	: register( t3 );

StructuredBuffer<AutoExposureInfos> AutoExposureBuffer : register( t8 );

cbuffer PassBuffer : register( b0 )
{
    float2  g_InputTextureDimension; //: packoffset( c0 );
    float   g_FilterSize;            //: packoffset( c0.z );
    int     g_SampleRadius;          //: packoffset( c0.w );
}

#if NYA_MSAA_X8
static const float2 g_SampleOffsets[8] = {
    float2(0.580f, 0.298f),
    float2(0.419f, 0.698f),
    float2(0.819f, 0.580f),
    float2(0.298f, 0.180f),
    float2(0.180f, 0.819f),
    float2(0.058f, 0.419f),
    float2(0.698f, 0.941f),
    float2(0.941f, 0.058f),
};
#define NYA_SAMPLER_COUNT 8
#elif NYA_MSAA_X4
static const float2 g_SampleOffsets[4] = {
    float2( 0.380f, 0.141f ),
    float2( 0.859f, 0.380f ),
    float2( 0.141f, 0.620f ),
    float2( 0.619f, 0.859f ),
};
#define NYA_SAMPLER_COUNT 4
#elif NYA_MSAA_X2
static const float2 g_SampleOffsets[2] = {
    float2( 0.741f, 0.741f ),
    float2( 0.258f, 0.258f ),
};
#define NYA_SAMPLER_COUNT 2
#else
static const float2 g_SampleOffsets[1] = {
    float2( 0.5f, 0.5f ),
};
#define NYA_SAMPLER_COUNT 1
#endif

float computeEV100FromAvgLuminance( float avg_luminance )
{
    return log2( avg_luminance * 100.0 / 12.5 );
}

float convertEV100ToExposure(float EV100)
{
    float maxLuminance = 1.2f * pow(2.0f, EV100);
    return 1.0f / maxLuminance;
}

float3 Reproject(in float2 pixelPos)
{
    // Find closest depth pixel (assuming input depth buffer is reversed)
    float2 velocity = 0.0f;
	float closestDepth = 0.0f;
	for(int vy = -1; vy <= 1; ++vy)
	{
		for(int vx = -1; vx <= 1; ++vx)
		{
			[unroll]
			for(uint vsIdx = 0; vsIdx < NYA_SAMPLER_COUNT; ++vsIdx)
			{
				float2 neighborVelocity = g_VelocityTexture.Load( pixelPos + int2( vx, vy ), vsIdx );
				float neighborDepth = g_DepthTexture.Load( pixelPos + int2( vx, vy ), vsIdx );
				
				if ( neighborDepth > closestDepth ) {
					velocity = neighborVelocity;
					closestDepth = neighborDepth;
				}
			}
		}
	}
  
    velocity *= g_InputTextureDimension;
    float2 reprojectedPos = pixelPos - velocity;
	
	float3 sum = 0.0f;
	float totalWeight = 0.0f;

    AutoExposureInfos currentExposure = AutoExposureBuffer[0];
	float currentEV = computeEV100FromAvgLuminance( currentExposure.EngineLuminanceFactor );
    
    float exposure = exp2( ( convertEV100ToExposure( currentEV ) ) );
    
	for(int ty = -1; ty <= 2; ++ty)
	{
		for(int tx = -1; tx <= 2; ++tx)
		{
			float2 samplePos = floor(reprojectedPos + float2(tx, ty)) + 0.5f;
			float3 reprojectedSample = g_LastFrameInputTexture[int2(samplePos)].xyz;

			float2 sampleDist = abs(samplePos - reprojectedPos);
			float filterWeight = ( 1.0f - smoothstep( 0.0f, 1.0f, sampleDist.x ) ) * ( 1.0f - smoothstep( 0.0f, 1.0f, sampleDist.y ) );

			float sampleLum = RGBToLuminance( reprojectedSample ) * exposure;
			filterWeight *= 1.0f / (1.0f + sampleLum);

			sum += reprojectedSample * filterWeight;
			totalWeight += filterWeight;
		}
	}

	return max(sum / totalWeight, 0.0f);
}

// From "Temporal Reprojection Anti-Aliasing"
// https://github.com/playdeadgames/temporal
float3 ClipAABB(float3 aabbMin, float3 aabbMax, float3 prevSample, float3 avg)
{
	// note: only clips towards aabb center (but fast!)
	float3 p_clip = 0.5 * (aabbMax + aabbMin);
	float3 e_clip = 0.5 * (aabbMax - aabbMin);

	float3 v_clip = prevSample - p_clip;
	float3 v_unit = v_clip.xyz / e_clip;
	float3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if (ma_unit > 1.0)
		return p_clip + v_clip / ma_unit;
	else
		return prevSample;// point inside aabb
}

float4 EntryPointPS( VertexStageData VertexStage ) : SV_TARGET0
{ 
    float2 pixelPos = VertexStage.Position.xy;
    float3 sum = 0.0f;
    float totalWeight = 0.0f;

    float3 clrMin = 99999999.0f;
    float3 clrMax = -99999999.0f;

    float3 m1 = 0.0f;
    float3 m2 = 0.0f;
    float mWeight = 0.0f;
	
    AutoExposureInfos currentExposure = AutoExposureBuffer[0];
	float currentEV = computeEV100FromAvgLuminance( currentExposure.EngineLuminanceFactor );
    
    float exposure = exp2( ( convertEV100ToExposure( currentEV ) ) );
    
    [loop]
    for ( int y = -g_SampleRadius; y <= g_SampleRadius; ++y ) {
        for ( int x = -g_SampleRadius; x <= g_SampleRadius; ++x ) {
            float2 sampleOffset = float2( x, y );
            float2 samplePos = pixelPos + sampleOffset;
            samplePos = clamp( samplePos, 0, g_InputTextureDimension - 1.0f );

            [unroll]
            for ( uint subSampleIdx = 0; subSampleIdx < NYA_SAMPLER_COUNT; ++subSampleIdx ) {
                sampleOffset += g_SampleOffsets[subSampleIdx] - 0.5f;
                float sampleDist = length( sampleOffset ) / ( g_FilterSize / 2.0f );

                [branch]
                if ( sampleDist <= 1.0f ) {
                    float3 sample = g_InputTexture.Load( samplePos, subSampleIdx ).xyz;
                    sample = max( sample, 0.0f );

                    float weight = 1.0f - smoothstep( 0.0f, 1.0f, sampleDist );
                    clrMin = min( clrMin, sample );
                    clrMax = max( clrMax, sample );

                    float sampleLum = RGBToLuminance( sample ) * exposure;

                    weight *= 1.0f / ( 1.0f + sampleLum );

                    sum += sample * weight;
                    totalWeight += weight;
					
					m1 += sample;
                    m2 += sample * sample;
                    mWeight += 1.0f;
                }
            }
        }
    }

    float3 output = sum / max( totalWeight, 0.00001f );
    output = max( output, 0.0f );

#if NYA_USE_TAA
    // NOTE TAA could have been done in a seprated pass, but this need resampling the resolved texture after a regular MSAA
    // resolve, which is pointless
    float3 currColor = output;
    float3 prevColor = Reproject( pixelPos );
	
	static const float VarianceClipGamma = 1.50f;
	float3 mu = m1 / mWeight;
	float3 sigma = sqrt(abs(m2 / mWeight - mu * mu));
	float3 minc = mu - VarianceClipGamma * sigma;
	float3 maxc = mu + VarianceClipGamma * sigma;
	prevColor = ClipAABB(minc, maxc, prevColor, mu);

    static const float TemporalAABlendFactor = 0.9000f;
    float3 weightA = saturate( 1.0f - TemporalAABlendFactor );
    float3 weightB = saturate( TemporalAABlendFactor );

    weightA *= 1.0f / ( 1.0f + RGBToLuminance( currColor ) );
    weightB *= 1.0f / ( 1.0f + RGBToLuminance( prevColor ) );

    output = ( currColor * weightA + prevColor * weightB ) / ( weightA + weightB );
#endif

    return float4( output, 1 );
}
