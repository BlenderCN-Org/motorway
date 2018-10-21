#include <Shared.hlsli>

static const int ThreadSize = 16;
static const uint NumVMFs = 1;

struct VMF
{
    float3 mu;
    float kappa;
    float alpha;
};

cbuffer Constants : register( b0 )
{
    float2 TextureSize;
    float2 OutputSize;
    uint MipLevel;
    float Roughness;
    float ScaleFactor;
};

// Inputs
Texture2D NormalMap : register( t0 );

#if FLAN_TEX_INPUT
Texture2D RoughnessMap : register( t1 );
#endif

// Outputs
RWTexture2D<unorm float> OutputRoughnessMap : register( u0 );

float3 FetchNormal(uint2 samplePos)
{
#if FLAN_TIGHT_TEXTURE_PACK
    float3 normal = normalize(RoughnessMap[samplePos].xyz * 2.0f - 1.0f);
#else
    float3 normal = normalize(NormalMap[samplePos].xyz * 2.0f - 1.0f);
#endif

    return normal;
}

float RoughnessToSpecPower(in float m) {
    return 2.0f / (m * m) - 2.0f;
}

// ================================================================================================
// Converts a Blinn-Phong specular power to a Beckmann roughness parameter
// ================================================================================================
float SpecPowerToRoughness(in float s) {
    return sqrt(2.0f / (s + 2.0f));
}

VMF SolveVMF( float2 centerPos, uint numSamples, out float vmfRoughness )
{
    VMF vmfs;

    if(MipLevel == 0)
    {
        vmfs.mu = FetchNormal(uint2(centerPos)).xyz,
        vmfs.alpha = 1.0f;
        vmfs.kappa = 10000.0f;

        [unroll]
        for(uint i = 1; i < NumVMFs; ++i)
        {
            vmfs.mu = 0.0f;
            vmfs.alpha = 0.0f;
            vmfs.kappa = 10000.0f;
        }

#if FLAN_TEX_INPUT
#if FLAN_TIGHT_TEXTURE_PACK
        vmfRoughness = RoughnessMap[uint2(centerPos)].a;
#else
        vmfRoughness = RoughnessMap[uint2(centerPos)].r;
#endif
#else
        vmfRoughness = Roughness;
#endif
    }
    else
    {
        float3 avgNormal = 0.0f;

#if FLAN_TEX_INPUT
        float avgRoughness = 0.0f;
#endif
        
        float2 topLeft = (-float(numSamples) / 2.0f) + 0.5f;

        for(uint y = 0; y < numSamples; ++y)
        {
            for(uint x = 0; x < numSamples; ++x)
            {
                float2 offset = topLeft + float2(x, y);
                float2 samplePos = floor(centerPos + offset) + 0.5f;
                float3 sampleNormal = FetchNormal(samplePos);
                
                avgNormal += sampleNormal;
                
#if FLAN_TEX_INPUT
#if FLAN_TIGHT_TEXTURE_PACK
                float sampleRoughness = RoughnessMap[samplePos].a;
#else
                float sampleRoughness = RoughnessMap[samplePos].r;
#endif
                avgRoughness += sampleRoughness;
#endif       
            }
        }

        avgNormal /= (numSamples * numSamples);
    
#if FLAN_TEX_INPUT
        avgRoughness /= (numSamples * numSamples);
#endif  

        float r = length(avgNormal);
        float kappa = 10000.0f;
        if(r < 1.0f)
            kappa = (3 * r - r * r * r) / (1 - r * r);

        float3 mu = normalize(avgNormal);

        vmfs.mu = mu;
        vmfs.alpha = 1.0f;
        vmfs.kappa = kappa;

        [unroll]
        for(uint i = 1; i < NumVMFs; ++i)
        {
            vmfs.mu = 0.0f;
            vmfs.alpha = 0.0f;
            vmfs.kappa = 10000.0f;
        }

        // Pre-compute roughness map values
#if FLAN_TEX_INPUT
        vmfRoughness = sqrt(avgRoughness * avgRoughness + (2.0f / kappa));
#else
        vmfRoughness = sqrt(Roughness * Roughness + (2.0f / kappa));
#endif
    }

    return vmfs;
}

[numthreads( ThreadSize, ThreadSize, 1 )]
void EntryPointCS( uint3 GroupID : SV_GroupID, uint3 DispatchThreadID : SV_DispatchThreadID,
              uint3 GroupThreadID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex )
{
    uint2 outputPos = GroupID.xy * uint2( ThreadSize, ThreadSize ) + GroupThreadID.xy;

    [branch]
    if( outputPos.x < uint( OutputSize.x ) 
     && outputPos.y < uint( OutputSize.y ) ) {
        float2 uv = (outputPos + 0.5f) / OutputSize;
        uint sampleRadius = (1 << MipLevel);
        float2 samplePos = uv * TextureSize;

        float vmfRoughness;
        VMF vmfs = SolveVMF( samplePos, sampleRadius, vmfRoughness );
        OutputRoughnessMap[outputPos] = sqrt( vmfRoughness );
    }
}
