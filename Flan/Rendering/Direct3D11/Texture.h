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

#if FLAN_D3D11
#include <Rendering/Texture.h>
#include <d3d11.h>

struct NativeRenderContext;
struct NativeCommandList;

struct NativeTextureObject
{
    union
    {
        ID3D11Texture1D*			texture1D;
        ID3D11Texture2D*			texture2D;
        ID3D11Texture3D*			texture3D;

        // In case the texture is actually loaded from disk
        ID3D11Resource*             textureResource;
    };

    ID3D11ShaderResourceView*		textureShaderResourceView;
    uint32_t textureWidth;
    uint32_t textureHeight;
    uint32_t textureMipCount;
    uint32_t textureArraySize;

    NativeTextureObject()
        : textureResource( nullptr )
        , textureShaderResourceView( nullptr )
        , textureWidth( 0 )
        , textureHeight( 0 )
        , textureMipCount( 0 )
        , textureArraySize( 0 )
    {

    }
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
        void CopySubresouceRegionAsynchronousImpl( NativeCommandList* nativeCmdList, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject, const uint32_t mipSrc = 0, const uint32_t arrayIdxSrc = 0, const uint32_t mipDst = 0, const uint32_t arrayIdxDst = 0 );
        void CopyResouceImpl( NativeRenderContext* nativeRenderContext, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject );
        void CopyResouceAsynchronousImpl( NativeCommandList* nativeCommandList, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject );

        void CopySubresouceRegionImpl( NativeRenderContext* nativeRenderContext, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject, const uint32_t mipSrc = 0, const uint32_t arrayIdxSrc = 0, const uint32_t mipDst = 0, const uint32_t arrayIdxDst = 0 );
        void SetTextureDebugNameImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const std::string& debugName );

        void RetrieveTextureTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<uint8_t>& texels );
        void RetrieveTextureLayerTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, const unsigned int layerIndex, const unsigned int mipLevel, std::vector<uint8_t>& texels );
        void RetrieveTextureTexelsHDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<float>& texels );

        void UpdateSubresourceImpl( NativeCommandList* nativeCommandList, NativeTextureObject* textureObject, const TextureCopyBox& copyBox, const uint32_t regionWidth, const uint32_t regionHeight, const uint32_t regionComposition, const void* regionData );
    }
}
#endif
