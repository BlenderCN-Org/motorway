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
#include <Rendering/Buffer.h>

#include "RenderContext.h"

struct NativeRenderContext;
struct NativeCommandList;
struct NativeTextureObject;

struct NativeBufferObject
{
    ID3D11Buffer*               bufferObject;
    ID3D11ShaderResourceView*   bufferResourceView;
    ID3D11UnorderedAccessView*  bufferUAVObject;
    BufferDesc::BufferType      bufferType;
    UINT                        stride;
    NativeTextureObject*        bufferTexture;

    NativeBufferObject()
        : bufferObject( nullptr )
        , bufferResourceView( nullptr )
        , bufferUAVObject( nullptr )
        , bufferType( BufferDesc::BufferType::UNKNOWN )
        , stride( 0 )
        , bufferTexture( nullptr )
    {

    }
};

namespace flan
{
    namespace rendering
    {
        NativeBufferObject* CreateBufferImpl( NativeRenderContext* nativeRenderContext, const BufferDesc& description, void* initialData = nullptr );
        void DestroyBufferImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject );

        void UpdateBufferImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize );
        void UpdateBufferAsynchronousImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize );
        void UpdateBufferRangeImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize, const int32_t bufferOffset = 0 );
        void FlushAndUpdateBufferRangeImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize, const int32_t bufferOffset = 0 );

        void BindBufferCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex );
        void BindBufferReadOnlyCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex );
        void UnbindBufferCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex, const Buffer::BindMode bindMode );
    
        void CopyStructureCountImpl( NativeCommandList* nativeCmdList, NativeBufferObject* sourceBufferObject, NativeBufferObject* destinationBufferObject, const uint32_t byteOffset = 0 );
    }
}
#endif
