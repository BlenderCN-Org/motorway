struct VertexStageData
{
	float4	Position : SV_POSITION;
	float2	TexCoordinates : TEXCOORD;
};

Texture2D 	g_InputTexture : register( t0 );
sampler		g_LinearSampler : register( s0 );

static const float A = 0.15;
static const float B = 0.50;
static const float C = 0.10;
static const float D = 0.20;
static const float E = 0.02;
static const float F = 0.30;
static const float W = 11.2;

float3 accurateLinearToSRGB( in float3 linearCol )
{
    float3 sRGBLo = linearCol * 12.92;
    float3 sRGBHi = ( pow( abs( linearCol ), 1.0 / 2.4 ) * 1.055 ) - 0.055;
    float3 sRGB = ( linearCol <= 0.0031308 ) ? sRGBLo : sRGBHi;
    return sRGB;
}

float3 Uncharted2Tonemap(float3 x)
{
     return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float3 InterleavedGradientNoise( float2 uv )
{
    const float3 magic = float3( 0.06711056, 0.00583715, 52.9829189 );
    return frac( magic.z * frac( dot( uv, magic.xy ) ) );
}

float4 EntryPointPS( VertexStageData VertexStage ) : SV_TARGET0
{
    float4 finalColor = g_InputTexture.Sample( g_LinearSampler, VertexStage.TexCoordinates );
    
    // Apply Tonemapping
    static const float ExposureBias = 2.0f;
    
    float3 curr = Uncharted2Tonemap(ExposureBias*finalColor.rgb * 2.0f);
    float3 whiteScale = 1.0f/Uncharted2Tonemap(W);
    float3 color = curr*whiteScale;
    
    color.rgb = accurateLinearToSRGB( color.rgb );
    
    // Add a dithering pattern to attenuate color banding
    float3 rnd = InterleavedGradientNoise( VertexStage.Position.xy ) / 255.0f;
    color.rgb += rnd;
        
    return float4( color, 1.0f );
}
