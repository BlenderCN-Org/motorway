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

#if NYA_VULKAN
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include "RenderDevice.h"
#include "CommandList.h"
#include "RenderTarget.h"

#include "Texture.h"
#include "ImageHelpers.h"

#include <vulkan/vulkan.h>

RenderTarget* RenderDevice::createRenderTarget1D( const TextureDescription& description, Texture* initialTexture )
{
    RenderTarget* renderTarget = nya::core::allocate<RenderTarget>( memoryAllocator );
    renderTarget->texture = ( initialTexture != nullptr ) ? initialTexture : createTexture1D( description );
    renderTarget->arraySize = description.arraySize;
    renderTarget->mipCount = description.mipCount;

    const VkImage image = renderTarget->texture->image;

    renderTarget->textureRenderTargetView = CreateImageView( renderContext->device, description, image );

    renderTarget->textureRenderTargetViewPerSlice = nya::core::allocateArray<VkImageView>( memoryAllocator, description.arraySize );
    renderTarget->textureRenderTargetViewPerSliceAndMipLevel = nya::core::allocateArray<VkImageView*>( memoryAllocator, description.arraySize );
    for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
        renderTarget->textureRenderTargetViewPerSlice[layerIdx] = CreateImageView( renderContext->device, description, image, layerIdx, 1u );

        renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx] = nya::core::allocateArray<VkImageView>( memoryAllocator, description.mipCount );
        for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
            renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateImageView( renderContext->device, description, image, layerIdx, 1u, mipLevel, 1u );
        }
    }

    return renderTarget;
}

RenderTarget* RenderDevice::createRenderTarget2D( const TextureDescription& description, Texture* initialTexture )
{
    RenderTarget* renderTarget = nya::core::allocate<RenderTarget>( memoryAllocator );
    renderTarget->texture = ( initialTexture != nullptr ) ? initialTexture : createTexture2D( description );
    renderTarget->arraySize = description.arraySize;
    renderTarget->mipCount = description.mipCount;

    const VkImage image = renderTarget->texture->image;

    renderTarget->textureRenderTargetView = CreateImageView( renderContext->device, description, image );

    renderTarget->textureRenderTargetViewPerSlice = nya::core::allocateArray<VkImageView>( memoryAllocator, description.arraySize );
    renderTarget->textureRenderTargetViewPerSliceAndMipLevel = nya::core::allocateArray<VkImageView*>( memoryAllocator, description.arraySize );
    for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
        renderTarget->textureRenderTargetViewPerSlice[layerIdx] = CreateImageView( renderContext->device, description, image, layerIdx, 1u );

        renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx] = nya::core::allocateArray<VkImageView>( memoryAllocator, description.mipCount );
        for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
            renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateImageView( renderContext->device, description, image, layerIdx, 1u, mipLevel, 1u );
        }
    }

    return renderTarget;
}

RenderTarget* RenderDevice::createRenderTarget3D( const TextureDescription& description, Texture* initialTexture )
{
    RenderTarget* renderTarget = nya::core::allocate<RenderTarget>( memoryAllocator );
    renderTarget->texture = ( initialTexture != nullptr ) ? initialTexture : createTexture3D( description );
    renderTarget->arraySize = description.arraySize;
    renderTarget->mipCount = description.mipCount;

    const VkImage image = renderTarget->texture->image;

    renderTarget->textureRenderTargetView = CreateImageView( renderContext->device, description, image );

    renderTarget->textureRenderTargetViewPerSlice = nya::core::allocateArray<VkImageView>( memoryAllocator, description.arraySize );
    renderTarget->textureRenderTargetViewPerSliceAndMipLevel = nya::core::allocateArray<VkImageView*>( memoryAllocator, description.arraySize );
    for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
        renderTarget->textureRenderTargetViewPerSlice[layerIdx] = CreateImageView( renderContext->device, description, image, layerIdx, 1u );

        renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx] = nya::core::allocateArray<VkImageView>( memoryAllocator, description.mipCount );
        for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
            renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateImageView( renderContext->device, description, image, layerIdx, 1u, mipLevel, 1u );
        }
    }

    return renderTarget;
}

void RenderDevice::destroyRenderTarget( RenderTarget* renderTarget )
{
    vkDestroyImageView( renderContext->device, renderTarget->textureRenderTargetView, nullptr );

    // Iterate and destroy rtv
    for ( unsigned int arrayIdx = 0u; arrayIdx < renderTarget->arraySize; arrayIdx++ ) {
        vkDestroyImageView( renderContext->device, renderTarget->textureRenderTargetViewPerSlice[arrayIdx], nullptr );

        for ( unsigned int mipIdx = 0u; mipIdx < renderTarget->mipCount; mipIdx++ ) {
            vkDestroyImageView( renderContext->device, renderTarget->textureRenderTargetViewPerSliceAndMipLevel[arrayIdx][mipIdx], nullptr );
        }
    }

    // Finally destroy bound texture
    destroyTexture( renderTarget->texture );

    nya::core::free( memoryAllocator, renderTarget );
}
#endif
