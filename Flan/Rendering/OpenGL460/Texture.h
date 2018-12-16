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

#if FLAN_GL460
#include <Rendering/Texture.h>
#include <vector>

#include "Extensions.h"
struct NativeRenderContext;
struct NativeCommandList;

struct NativeTextureObject
{
    NativeTextureObject()
        : textureHandle( 0 )
        , target( 0 )
    {

    }

    GLuint  textureHandle;
    GLenum  target;
};

namespace flan
{
    namespace rendering
    {
        NativeTextureObject* CreateTexture1DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );
        NativeTextureObject* CreateTexture2DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );
        NativeTextureObject* CreateTexture3DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );

        void DestroyTextureImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject );
        void BindTextureCmdImpl( NativeCommandList* nativeCmdList, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo );
        void UnbindTextureCmdImpl( NativeCommandList* nativeCmdList, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo );

        void SetTextureDebugNameImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const std::string& debugName );

        void RetrieveTextureTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<uint8_t>& texels );
        void RetrieveTextureLayerTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, const unsigned int layerIndex, const unsigned int mipLevel, std::vector<uint8_t>& texels );
        void RetrieveTextureTexelsHDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<float>& texels );
        void CopySubresouceRegionAsynchronousImpl( NativeCommandList* nativeCmdList, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject, const uint32_t mipSrc = 0, const uint32_t arrayIdxSrc = 0, const uint32_t mipDst = 0, const uint32_t arrayIdxDst = 0 );
        void CopyResouceImpl( NativeRenderContext* nativeRenderContext, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject );
        void CopyResouceAsynchronousImpl( NativeCommandList* nativeCommandList, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject );

        void CopySubresouceRegionImpl( NativeRenderContext* nativeRenderContext, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject, const uint32_t mipSrc = 0, const uint32_t arrayIdxSrc = 0, const uint32_t mipDst = 0, const uint32_t arrayIdxDst = 0 );

        void UpdateSubresourceImpl( NativeCommandList* nativeCommandList, NativeTextureObject* textureObject, const TextureCopyBox& copyBox, const uint32_t regionWidth, const uint32_t regionHeight, const uint32_t regionComposition, const void* regionData );
        void CopyRenderTargetImpl( NativeCommandList* nativeCmdList, NativeTextureObject* source, NativeTextureObject* dest );
    }
}
#endif
