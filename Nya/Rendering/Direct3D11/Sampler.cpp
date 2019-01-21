/*
    Project Motorway Source Code
    Copyright (C) 2018 Prévost Baptiste

    This file is part of Project Motorway source code.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <Shared.h>

#if NYA_D3D11
#include <Rendering/RenderDevice.h>
#include "RenderDevice.h"

#include "Sampler.h"

#include "ComparisonFunctions.h"

#include <d3d11.h>

using namespace nya::rendering;

static constexpr D3D11_TEXTURE_ADDRESS_MODE D3D11_SAMPLER_ADDRESS[eSamplerAddress::SAMPLER_ADDRESS_COUNT] = {
    D3D11_TEXTURE_ADDRESS_WRAP,
    D3D11_TEXTURE_ADDRESS_MIRROR,
    D3D11_TEXTURE_ADDRESS_CLAMP,
    D3D11_TEXTURE_ADDRESS_BORDER,
    D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
};

static constexpr D3D11_FILTER D3D11_SAMPLER_FILTER[eSamplerFilter::SAMPLER_FILTER_COUNT] = {
    D3D11_FILTER_MIN_MAG_MIP_POINT,
    D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MIN_MAG_MIP_LINEAR,
    D3D11_FILTER_ANISOTROPIC,
    D3D11_FILTER_ANISOTROPIC,

    D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_ANISOTROPIC,
    D3D11_FILTER_COMPARISON_ANISOTROPIC,
};

Sampler* RenderDevice::createSampler( const SamplerDesc& description )
{
    D3D11_SAMPLER_DESC samplerDesc = {
        D3D11_SAMPLER_FILTER[description.filter],
        D3D11_SAMPLER_ADDRESS[description.addressU],
        D3D11_SAMPLER_ADDRESS[description.addressV],
        D3D11_SAMPLER_ADDRESS[description.addressW]
    };

    if ( description.filter == SAMPLER_FILTER_ANISOTROPIC_8 )
        samplerDesc.MaxAnisotropy = 8;
    else if ( description.filter == SAMPLER_FILTER_ANISOTROPIC_16 )
        samplerDesc.MaxAnisotropy = 16;

    samplerDesc.MinLOD = static_cast<FLOAT>( description.minLOD );
    samplerDesc.MaxLOD = static_cast<FLOAT>( description.maxLOD );
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNCTION[description.comparisonFunction];
    samplerDesc.BorderColor[0] = 1.0F;
    samplerDesc.BorderColor[1] = 1.0F;
    samplerDesc.BorderColor[2] = 1.0F;
    samplerDesc.BorderColor[3] = 1.0F;

    ID3D11Device* nativeDevice = renderContext->nativeDevice;
  
    Sampler* sampler = nya::core::allocate<Sampler>( memoryAllocator );
    nativeDevice->CreateSamplerState( &samplerDesc, &sampler->samplerState );

    return sampler;
}

void RenderDevice::destroySampler( Sampler* sampler )
{
   sampler->samplerState->Release();

    nya::core::free( memoryAllocator, sampler );
}
#endif
