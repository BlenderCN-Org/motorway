#ifndef __TESS_H__
#define __TESS_H__
float CalcTessFactor(float3 p)
{
    static const float gMinDist = 8.0f;
    static const float gMaxDist = 256.0f;

    static const float gMaxTess = 6.0f;
    static const float gMinTess = 1.0f;
    
    float d = distance(p, WorldPosition);

   // return saturate((gMaxDist - d) / gMaxDist)*64.0f;
    
    float s = saturate( (d - gMinDist) / (gMaxDist - gMinDist) );
    
    return pow(2, ( lerp( gMaxTess, gMinTess, s ) ) );
}
#endif
