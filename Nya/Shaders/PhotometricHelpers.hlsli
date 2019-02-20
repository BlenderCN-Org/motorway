// RGB to luminance
inline float RGBToLuminance( const in float3 linearRGB )
{
    return dot( float3( 0.2126f, 0.7152f, 0.0722f ), linearRGB );
}
