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
#include "RenderTarget.h"

#include "RenderDevice.h"
#include "CommandList.h"

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#include "OpenGL460/Texture.h"
#include "OpenGL460/RenderTarget.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#include "Direct3D11/Texture.h"
#include "Direct3D11/RenderTarget.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#include "Vulkan/CommandList.h"
#include "Vulkan/RenderTarget.h"
#include "Vulkan/Texture.h"
#endif

RenderTarget::RenderTarget()
    : textureDescription{}
    , nativeTextureObject( nullptr )
    , nativeRenderTargetObject( nullptr )
    , shaderStagesBindTo( 0 )
    , bindingIndex( 0 )
{

}

RenderTarget::~RenderTarget()
{
    textureDescription = {};
    shaderStagesBindTo = 0;
    bindingIndex = 0;
}

void RenderTarget::createAsRenderTarget1D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    textureDescription = description;

    nativeTextureObject.reset( flan::rendering::CreateTexture1DImpl( renderDevice->getNativeRenderContext(), textureDescription, initialData, initialDataSize ) );
    nativeRenderTargetObject.reset( flan::rendering::CreateRenderTarget1DImpl( renderDevice->getNativeRenderContext(), textureDescription, nativeTextureObject.get() ) );
}

void RenderTarget::createAsRenderTarget2D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    textureDescription = description;

    nativeTextureObject.reset( flan::rendering::CreateTexture2DImpl( renderDevice->getNativeRenderContext(), textureDescription, initialData, initialDataSize ) );
    nativeRenderTargetObject.reset( flan::rendering::CreateRenderTarget2DImpl( renderDevice->getNativeRenderContext(), textureDescription, nativeTextureObject.get() ) );
}

void RenderTarget::createAsRenderTarget3D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    textureDescription = description;

    nativeTextureObject.reset( flan::rendering::CreateTexture3DImpl( renderDevice->getNativeRenderContext(), textureDescription, initialData, initialDataSize ) );
    nativeRenderTargetObject.reset( flan::rendering::CreateRenderTarget3DImpl( renderDevice->getNativeRenderContext(), textureDescription, nativeTextureObject.get() ) );
}

void RenderTarget::destroy( RenderDevice* renderDevice )
{
    flan::rendering::DestroyTextureImpl( renderDevice->getNativeRenderContext(), nativeTextureObject.get() );
    flan::rendering::DestroyRenderTargetImpl( renderDevice->getNativeRenderContext(), nativeRenderTargetObject.get() );
}

void RenderTarget::bind( CommandList* cmdList, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
    this->bindingIndex = bindingIndex;
    this->shaderStagesBindTo = shaderStagesToBindTo;

    flan::rendering::BindTextureCmdImpl( cmdList->getNativeCommandList(), nativeTextureObject.get(), bindingIndex, shaderStagesToBindTo );
}

void RenderTarget::unbind( CommandList* cmdList )
{
    flan::rendering::UnbindTextureCmdImpl( cmdList->getNativeCommandList(), nativeTextureObject.get(), this->bindingIndex, this->shaderStagesBindTo );

    this->bindingIndex = 0;
    this->shaderStagesBindTo = 0;
}

TextureDescription::Dimension RenderTarget::getTextureDimension() const
{
    return textureDescription.dimension;
}

NativeTextureObject* RenderTarget::getNativeTextureObject() const
{
    return nativeTextureObject.get();
}

NativeRenderTargetObject* RenderTarget::getNativeRenderTargetObject() const
{
    return nativeRenderTargetObject.get();
}

const TextureDescription& RenderTarget::getDescription() const
{
    return textureDescription;
}

void RenderTarget::retrieveTexelsLDR( RenderDevice* renderDevice, std::vector<uint8_t>& texels )
{
    flan::rendering::RetrieveTextureTexelsLDRImpl( renderDevice->getNativeRenderContext(), nativeTextureObject.get(), textureDescription, texels );
}

void RenderTarget::retrieveLayerTexelsLDR( RenderDevice* renderDevice, const unsigned int layerIndex, const unsigned int mipLevel, std::vector<uint8_t>& texels )
{
    flan::rendering::RetrieveTextureLayerTexelsLDRImpl( renderDevice->getNativeRenderContext(), nativeTextureObject.get(), textureDescription, layerIndex, mipLevel, texels );
}

void RenderTarget::retrieveTexelsHDR( RenderDevice* renderDevice, std::vector<float>& texels )
{
    flan::rendering::RetrieveTextureTexelsHDRImpl( renderDevice->getNativeRenderContext(), nativeTextureObject.get(), textureDescription, texels );
}
