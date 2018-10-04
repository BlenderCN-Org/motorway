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
#include "Texture.h"

#include "RenderDevice.h"
#include "CommandList.h"

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#include "OpenGL460/Texture.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#include "Direct3D11/CommandList.h"
#include "Direct3D11/Texture.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#include "Vulkan/CommandList.h"
#include "Vulkan/Texture.h"
#endif

Texture::Texture()
    : textureDescription{}
    , nativeTextureObject( nullptr )
    , resourceName( "Texture" )
    , bindingIndex( 0 )
    , shaderStagesBindTo( 0 )
{

}

Texture::~Texture()
{
    textureDescription = {};
    resourceName.clear();
    bindingIndex = 0;
    shaderStagesBindTo = 0;
}

void Texture::createAsTexture1D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    textureDescription = description;

    nativeTextureObject.reset( flan::rendering::CreateTexture1DImpl( renderDevice->getNativeRenderContext(), textureDescription, initialData, initialDataSize ) );
}

void Texture::createAsTexture2D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    textureDescription = description;

    nativeTextureObject.reset( flan::rendering::CreateTexture2DImpl( renderDevice->getNativeRenderContext(), textureDescription, initialData, initialDataSize ) );
}

void Texture::createAsTexture3D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    textureDescription = description;

    nativeTextureObject.reset( flan::rendering::CreateTexture3DImpl( renderDevice->getNativeRenderContext(), textureDescription, initialData, initialDataSize ) );
}

void Texture::destroy( RenderDevice* renderDevice )
{
    flan::rendering::DestroyTextureImpl( renderDevice->getNativeRenderContext(), nativeTextureObject.get() );
}

void Texture::bind( CommandList* cmdList, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
    this->bindingIndex = bindingIndex;
    this->shaderStagesBindTo = shaderStagesToBindTo;

    flan::rendering::BindTextureCmdImpl( cmdList->getNativeCommandList(), nativeTextureObject.get(), bindingIndex, shaderStagesToBindTo );
}

void Texture::unbind( CommandList* cmdList )
{
    flan::rendering::UnbindTextureCmdImpl( cmdList->getNativeCommandList(), nativeTextureObject.get(), this->bindingIndex, this->shaderStagesBindTo );

    this->bindingIndex = 0;
    this->shaderStagesBindTo = 0;
}

void Texture::setResourceName( RenderDevice* renderDevice, const std::string& name )
{
    resourceName = name;

    flan::rendering::SetTextureDebugNameImpl( renderDevice->getNativeRenderContext(), nativeTextureObject.get(), name );
}

const std::string& Texture::getResourceName() const
{
    return resourceName;
}

const TextureDescription& Texture::getDescription() const
{
    return textureDescription;
}

const NativeTextureObject* Texture::getNativeObject() const
{
    return nativeTextureObject.get();
}

void Texture::retrieveTexelsLDR( RenderDevice* renderDevice, std::vector<uint8_t>& texels )
{
    flan::rendering::RetrieveTextureTexelsLDRImpl( renderDevice->getNativeRenderContext(), nativeTextureObject.get(), textureDescription, texels );
}

void Texture::retrieveTexelsHDR( RenderDevice* renderDevice, std::vector<float>& texels )
{
    flan::rendering::RetrieveTextureTexelsHDRImpl( renderDevice->getNativeRenderContext(), nativeTextureObject.get(), textureDescription, texels );
}

void Texture::copyResource( RenderDevice* renderDevice, const Texture* resourceToCopy )
{
    flan::rendering::CopyResouceImpl( renderDevice->getNativeRenderContext(), resourceToCopy->getNativeObject(), getNativeObject() );
}

void Texture::copyResourceAsynchronous( CommandList* cmdList, const Texture* resourceToCopy )
{
    flan::rendering::CopyResouceAsynchronousImpl( cmdList->getNativeCommandList(), resourceToCopy->getNativeObject(), getNativeObject() );
}

void Texture::copySubresource( RenderDevice* renderDevice, const Texture* resourceToCopy, const uint32_t mipSrc, const uint32_t arrayIdxSrc, const uint32_t mipDst, const uint32_t arrayIdxDst )
{
    flan::rendering::CopySubresouceRegionImpl( renderDevice->getNativeRenderContext(), resourceToCopy->getNativeObject(), getNativeObject(), mipSrc, arrayIdxSrc, mipDst, arrayIdxDst );
}

void Texture::copySubresourceAsynchronous( CommandList* cmdList, const Texture* resourceToCopy, const uint32_t mipSrc, const uint32_t arrayIdxSrc, const uint32_t mipDst, const uint32_t arrayIdxDst )
{
    flan::rendering::CopySubresouceRegionAsynchronousImpl( cmdList->getNativeCommandList(), resourceToCopy->getNativeObject(), getNativeObject() );
}
