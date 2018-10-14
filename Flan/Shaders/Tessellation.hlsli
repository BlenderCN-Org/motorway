#ifndef __TESS_H__
#define __TESS_H__
float CalcTessFactor(float3 p)
{
    static const float gMinDist = 0.10f;
    static const float gMaxDist = 512.0f;

    static const int gMaxTess = 6;
    static const int gMinTess = 0;
    
    float d = distance(p, WorldPosition);

    float s = saturate( (d - gMinDist) / (gMaxDist - gMinDist) );
    
    return pow(2, (lerp(gMaxTess, gMinTess, s)) );
}
#endif
