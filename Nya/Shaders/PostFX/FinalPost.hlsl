#include <AutoExposure/SharedAutoExposure.hlsli>

float3 accurateLinearToSRGB( in float3 linearCol )
{
    float3 sRGBLo = linearCol * 12.92;
    float3 sRGBHi = ( pow( abs( linearCol ), 1.0 / 2.4 ) * 1.055 ) - 0.055;
    float3 sRGB = ( linearCol <= 0.0031308 ) ? sRGBLo : sRGBHi;
    return sRGB;
}
// Academy Color Encoding System [http://www.oscars.org/science-technology/sci-tech-projects/aces]
float3 ACESFilmic(const float3 x) 
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float3 Uncharted2Tonemap(float3 x)
{
    static const float A = 0.15;
    static const float B = 0.50;
    static const float C = 0.10;
    static const float D = 0.20;
    static const float E = 0.02;
    static const float F = 0.30;
    static const float W = 11.2;

    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float3 InterleavedGradientNoise( float2 uv )
{
    const float3 magic = float3( 0.06711056, 0.00583715, 52.9829189 );
    return frac( magic.z * frac( dot( uv, magic.xy ) ) );
}

float3 computeBloomLuminance( float3 bloomColor, float bloomEC, float currentEV )
{
    // currentEV is the value calculated at the previous frame
    float bloomEV = currentEV + bloomEC;

    // convert to luminance
    // See equation (12) for explanation about converting EV to luminance
    return bloomColor * pow( 2, bloomEV - 3 );
}

float computeEV100FromAvgLuminance( float avg_luminance )
{
    return log2( avg_luminance * 100.0 / 12.5 );
}

float convertEV100ToExposure(float EV100)
{
    float maxLuminance = 1.2f * pow(2.0f, EV100);
    return 1.0f / maxLuminance;
}

Texture2D<float4>	g_InputRenderTarget : register( t0 );
Texture2D<float3>	g_BloomRenderTarget : register( t1 );

StructuredBuffer<AutoExposureInfos> AutoExposureBuffer : register( t8 );

RWTexture2D<float4>	g_OutputRenderTarget : register( u0 );

[numthreads( 16, 16, 1 )]
void EntryPointCS( uint2 id : SV_DispatchThreadID )
{
    static const float g_BloomStrength = 0.00001f;
       
    AutoExposureInfos currentExposure = AutoExposureBuffer[0];
    float currentEV = computeEV100FromAvgLuminance( currentExposure.EngineLuminanceFactor );
   
    float exposure = exp2( ( convertEV100ToExposure( currentEV ) ) );
    
	float4 finalColor = g_InputRenderTarget.Load( int3( id, 0 ) );    
	float3 bloomColor = g_BloomRenderTarget.Load( int3( id, 0 ) );
    
    float3 bloomLuminance = computeBloomLuminance( bloomColor, 0.0f, currentEV );
    finalColor.rgb = lerp( finalColor.rgb, bloomLuminance, g_BloomStrength );
    
    float3 color = ACESFilmic( finalColor.rgb * exposure );
    
    color = accurateLinearToSRGB( color );

    // Add a dithering pattern to attenuate color banding
    float3 rnd = InterleavedGradientNoise( float2( id ) ) / 255.0f;
    color.rgb += rnd;
   
	g_OutputRenderTarget[id] = float4( color, 1.0f );
}
