
struct MaterialLayer
{
    float3  BaseColor;
    float   Reflectance;
    
    float3  Normal;
    float   Roughness;
    
    float   Metalness;
    float   AmbientOcclusion;
    float2  LayerScale;
    
    float   Emissivity;    
    float   AlphaMask;
    float   Refraction;
    float   RefractionIor;
	
	float3	EXPICIT__PADDING;
    float   AlphaCutoff;
};

#define PA_EDITOR 1
#if PA_EDITOR
struct MaterialEditionInput
{
    // Input can have 4 states:
    //      0: none
    //      1: constant 1d value
    //      2: constant 3d vector    
    //      3: texture input
    float3  Input3D;
    float   Input1D;
    
    int     Type;
    uint    SamplingChannel; // (default = 0 = red)
    float2  EXPLICIT_PADDING;
};

cbuffer MaterialEdition : register( b8 )
{
    // Flags
    uint                    g_WriteVelocity; // Range: 0..1 (should be 0 for transparent surfaces)
    uint                    g_EnableAlphaTest;
    uint                    g_EnableAlphaBlend;
    uint                    g_IsDoubleFace;
    
    uint                    g_CastShadow;
    uint                    g_ReceiveShadow;
    uint                    g_EnableAlphaToCoverage;
    float   				g_AlphaCutoff;    
    
    MaterialEditionInput   	g_BaseColor;
    MaterialEditionInput   	g_Reflectance;   
    MaterialEditionInput   	g_Normal;
    MaterialEditionInput   	g_Roughness;
    
    MaterialEditionInput   	g_Metalness;
    MaterialEditionInput   	g_AmbientOcclusion;
    MaterialEditionInput   	g_Emissivity;
    MaterialEditionInput   	g_AlphaMask;
    
    float2					g_LayerScale;
    float   				g_Refraction;
    float   				g_RefractionIor;
};
#endif

#define PA_READ_INPUT_1D( input, defaultValue )\
float input = defaultValue;\
if ( g_##input.Type == 1 )\
    input = g_##input##.Input1D;\
else if ( g_##input##.Type == 2 )\
    input = g_##input##.Input3D.r;\
else if ( g_##input##.Type == 3 )\
    input = g_Tex##input##.Sample( g_GeometrySampler, VertexStage.uvCoord * g_LayerScale ).r;

#define PA_READ_INPUT_3D_SRGB( input, defaultValue )\
float3 input = defaultValue;\
if ( g_##input.Type == 1 )\
    input = float3( g_##input##.Input1D, g_##input##.Input1D, g_##input.Input1D );\
else if ( g_##input##.Type == 2 )\
    input = g_##input##.Input3D;\
else if ( g_##input##.Type == 3 )\
    input = accurateSRGBToLinear( g_Tex##input##.Sample( g_GeometrySampler, VertexStage.uvCoord * g_LayerScale ).rgb );

#define PA_READ_INPUT_3D( input, defaultValue )\
float3 input = defaultValue;\
if ( g_##input.Type == 1 )\
    input = float3( g_##input##.Input1D, g_##input##.Input1D, g_##input.Input1D );\
else if ( g_##input##.Type == 2 )\
    input = g_##input##.Input3D;\
else if ( g_##input##.Type == 3 )\
    input = g_Tex##input##.Sample( g_GeometrySampler, VertexStage.uvCoord * g_LayerScale ).rgb;

float ReadInput1D( in float2 uvCoord, in MaterialEditionInput materialInput, in Texture2D materialInputTex, const float defaultValue )
{
    float input = defaultValue;
        
    if ( materialInput.Type == 1 ) input = materialInput.Input1D;
    else if ( materialInput.Type == 2 ) input = materialInput.Input3D.r;
    else if ( materialInput.Type == 3 ) input = materialInputTex.Sample( g_GeometrySampler, uvCoord * g_LayerScale ).r;

    return input;
}

float3 ReadInput3D( in float2 uvCoord, in MaterialEditionInput materialInput, in Texture2D materialInputTex, const float3 defaultValue )
{
    float3 input = defaultValue;
        
    if ( materialInput.Type == 1 ) input = materialInput.Input1D.rrr;
    else if ( materialInput.Type == 2 ) input = materialInput.Input3D.rgb;
    else if ( materialInput.Type == 3 ) input = materialInputTex.Sample( g_GeometrySampler, uvCoord * g_LayerScale ).rgb;

    return input;
}

float3 ReadInput3DSRGB( in float2 uvCoord, in MaterialEditionInput materialInput, in Texture2D materialInputTex, const float3 defaultValue )
{
    float3 input = defaultValue;
        
    if ( materialInput.Type == 1 ) input = materialInput.Input1D.rrr;
    else if ( materialInput.Type == 2 ) input = materialInput.Input3D.rgb;
    else if ( materialInput.Type == 3 ) input = materialInputTex.Sample( g_GeometrySampler, uvCoord * g_LayerScale ).rgb;

    input = accurateSRGBToLinear( input );
    
    return input;
}

Texture2D g_TexBaseColor : register( t17 );
Texture2D g_TexReflectance : register( t18 );
Texture2D g_TexRoughness : register( t19 );
Texture2D g_TexMetalness : register( t20 );
Texture2D g_TexAmbientOcclusion : register( t21 );
Texture2D g_TexNormal : register( t22 );
Texture2D g_TexEmissivity : register( t23 );
Texture2D g_TexAlphaMask : register( t24 );
