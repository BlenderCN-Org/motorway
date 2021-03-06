/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

#if NYA_VULKAN
#include <Rendering/RenderDevice.h>
#include "RenderDevice.h"

#include "Sampler.h"

#include "ComparisonFunctions.h"

#include <vulkan/vulkan.h>

using namespace nya::rendering;

static constexpr VkSamplerAddressMode VK_SAMPLER_ADDRESS[eSamplerAddress::SAMPLER_ADDRESS_COUNT] = {
    VK_SAMPLER_ADDRESS_MODE_REPEAT,
    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
};

static constexpr VkFilter VK_MIN_FILTER[eSamplerFilter::SAMPLER_FILTER_COUNT] = {
    VK_FILTER_NEAREST,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,

    VK_FILTER_NEAREST,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
};

static constexpr VkFilter VK_MAG_FILTER[eSamplerFilter::SAMPLER_FILTER_COUNT] = {
    VK_FILTER_NEAREST,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,

    VK_FILTER_NEAREST,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
    VK_FILTER_LINEAR,
};

static constexpr VkSamplerMipmapMode VK_MIP_MAP_MODE[eSamplerFilter::SAMPLER_FILTER_COUNT] = {
    VK_SAMPLER_MIPMAP_MODE_NEAREST,
    VK_SAMPLER_MIPMAP_MODE_LINEAR,
    VK_SAMPLER_MIPMAP_MODE_LINEAR,
    VK_SAMPLER_MIPMAP_MODE_LINEAR,
    VK_SAMPLER_MIPMAP_MODE_LINEAR,

    VK_SAMPLER_MIPMAP_MODE_NEAREST,
    VK_SAMPLER_MIPMAP_MODE_LINEAR,
    VK_SAMPLER_MIPMAP_MODE_LINEAR,
    VK_SAMPLER_MIPMAP_MODE_LINEAR,
    VK_SAMPLER_MIPMAP_MODE_LINEAR,
};

Sampler* RenderDevice::createSampler( const SamplerDesc& description )
{
    const bool useAnisotropicFiltering = ( description.filter == eSamplerFilter::SAMPLER_FILTER_ANISOTROPIC_16
        || description.filter == eSamplerFilter::SAMPLER_FILTER_ANISOTROPIC_8
        || description.filter == eSamplerFilter::SAMPLER_FILTER_COMPARISON_ANISOTROPIC_8
        || description.filter == eSamplerFilter::SAMPLER_FILTER_COMPARISON_ANISOTROPIC_16 );

    const bool useComparisonFunction = ( description.comparisonFunction != eComparisonFunction::COMPARISON_FUNCTION_ALWAYS );

    // Build object descriptor
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_MAG_FILTER[description.filter];
    samplerInfo.minFilter = VK_MIN_FILTER[description.filter];
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS[description.addressU];
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS[description.addressV];
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS[description.addressW];
    samplerInfo.anisotropyEnable = static_cast< VkBool32 >( useAnisotropicFiltering );
    samplerInfo.maxAnisotropy = ( description.filter == eSamplerFilter::SAMPLER_FILTER_COMPARISON_ANISOTROPIC_8 ) ? 8.0f : 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = static_cast< VkBool32 >( useComparisonFunction );
    samplerInfo.compareOp = VK_COMPARISON_FUNCTION[description.comparisonFunction];
    samplerInfo.mipmapMode = VK_MIP_MAP_MODE[description.filter];
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = static_cast<float>( description.minLOD );
    samplerInfo.maxLod = static_cast<float>( description.maxLOD );

    VkSampler samplerObject;

    VkResult operationResult = vkCreateSampler( renderContext->device, &samplerInfo, nullptr, &samplerObject );
    if ( operationResult != VK_SUCCESS ) {
        NYA_CERR << "Sampler creation failed! (error code: " << operationResult << ")" << std::endl;
        return nullptr;
    }

    Sampler* sampler = nya::core::allocate<Sampler>( memoryAllocator );
    sampler->samplerState = samplerObject;

    return sampler;
}

void RenderDevice::destroySampler( Sampler* sampler )
{
    vkDestroySampler( renderContext->device, sampler->samplerState, nullptr );

    nya::core::free( memoryAllocator, sampler );
}
#endif
