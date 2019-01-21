#pragma once

#if NYA_D3D11
struct ID3D11SamplerState;

struct Sampler
{
    ID3D11SamplerState* samplerState;
};
#endif
