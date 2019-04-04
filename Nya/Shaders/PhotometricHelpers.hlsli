// RGB to luminance
inline float RGBToLuminance( const in float3 linearRGB )
{
    return dot( float3( 0.2126f, 0.7152f, 0.0722f ), linearRGB );
}

static const float MaxRange = 65025.0f;

float4 EncodeRGBD(float3 rgb)
{
    float maxRGB = max(rgb.r,max(rgb.g,rgb.b));
    float D      = max(MaxRange / maxRGB, 1.0f);
    D            = saturate(floor(D) / 255.0f);
    return float4(rgb.rgb * (D * (255.0f / MaxRange)), D);
}

float3 DecodeRGBD(float4 rgbd)
{
    return rgbd.rgb * ((MaxRange / 255.0f) / rgbd.a);
}

float3 accurateSRGBToLinear( in float3 sRGBCol )
{
    float3 linearRGBLo = sRGBCol / 12.92;

    // Ignore X3571, incoming vector will always be superior to 0
    float3 linearRGBHi = pow( abs( ( sRGBCol + 0.055 ) / 1.055 ), 2.4 );

    float3 linearRGB = ( sRGBCol <= 0.04045 ) ? linearRGBLo : linearRGBHi;

    return linearRGB;
}
