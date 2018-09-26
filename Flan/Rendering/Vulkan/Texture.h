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
#pragma once

#if FLAN_VULKAN
#include <Rendering/Texture.h>

struct NativeRenderContext;
struct NativeCommandList;

struct NativeTextureObject
{
};

namespace flan
{
    namespace rendering
    {
        NativeTextureObject* CreateTexture1DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );
        NativeTextureObject* CreateTexture2DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );
        NativeTextureObject* CreateTexture3DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );

        void DestroyTextureImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject );
        void BindTextureImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo );
        void UnbindTextureImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo );
        void BindTextureCmdImpl( NativeCommandList* nativeCmdList, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo );
        void UnbindTextureCmdImpl( NativeCommandList* nativeCmdList, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo );

        void SetTextureDebugNameImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const std::string& debugName );

        void RetrieveTextureTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<uint8_t>& texels );
        void RetrieveTextureLayerTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, const unsigned int layerIndex, const unsigned int mipLevel, std::vector<uint8_t>& texels );
        void RetrieveTextureTexelsHDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<float>& texels );
    }
}
#endif
