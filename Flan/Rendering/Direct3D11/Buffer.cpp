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

#if FLAN_D3D11
#include "Buffer.h"
#include "CommandList.h"

#include "Texture.h"

NativeBufferObject* flan::rendering::CreateBufferImpl( NativeRenderContext* nativeRenderContext, const BufferDesc& description, void* initialData )
{
    // Buffer object description
    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.Size );

    // Bind Flags
    UINT bindFlags = 0;
    if ( description.Type == BufferDesc::CONSTANT_BUFFER ) {
        bindFlags |= D3D11_BIND_CONSTANT_BUFFER;

        bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
    } else if ( description.Type == BufferDesc::DYNAMIC_VERTEX_BUFFER ) {
        bindFlags |= D3D11_BIND_VERTEX_BUFFER;

        bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
    } else if ( description.Type == BufferDesc::VERTEX_BUFFER ) {
        bindFlags |= D3D11_BIND_VERTEX_BUFFER;

        bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    } else if ( description.Type == BufferDesc::DYNAMIC_INDICE_BUFFER ) {
        bindFlags |= D3D11_BIND_INDEX_BUFFER;

        bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
    } else if ( description.Type == BufferDesc::INDICE_BUFFER ) {
        bindFlags |= D3D11_BIND_INDEX_BUFFER;

        bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    } else {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;

        bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    }

    if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER
    || description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_1D
    || description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D
    || description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D
    || description.Type == BufferDesc::STRUCTURED_BUFFER 
    || description.Type == BufferDesc::APPEND_STRUCTURED_BUFFER
    || description.Type == BufferDesc::INDIRECT_DRAW_ARGUMENTS ) {
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }

    bufferDescription.BindFlags = bindFlags;

    DXGI_FORMAT nativeTextureFormat = static_cast< DXGI_FORMAT >( description.ViewFormat );

    if ( description.Type == BufferDesc::STRUCTURED_BUFFER
      || description.Type == BufferDesc::APPEND_STRUCTURED_BUFFER ) {
        bufferDescription.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDescription.StructureByteStride = static_cast<UINT>( description.Size );
        bufferDescription.ByteWidth *= static_cast<UINT>( description.Stride );

        nativeTextureFormat = DXGI_FORMAT_UNKNOWN;
    } else if ( description.Type == BufferDesc::INDIRECT_DRAW_ARGUMENTS ) {
        bufferDescription.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

        bufferDescription.StructureByteStride = static_cast<UINT>( description.Size );
        bufferDescription.ByteWidth *= static_cast<UINT>( description.Stride );
    }

    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    // Shader Resource View Description (for everything except constant buffer)
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
        nativeTextureFormat,
        D3D11_SRV_DIMENSION_BUFFER
    };

    if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D ) {
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;

        shaderResourceViewDesc.Texture2D.MipLevels = -1;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    } else if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D ) {
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE3D;

        shaderResourceViewDesc.Texture3D.MostDetailedMip = 0;
        shaderResourceViewDesc.Texture3D.MipLevels = -1;
    } else if ( description.Type != BufferDesc::VERTEX_BUFFER || description.Type != BufferDesc::DYNAMIC_VERTEX_BUFFER
        || description.Type != BufferDesc::INDICE_BUFFER || description.Type != BufferDesc::DYNAMIC_INDICE_BUFFER ) {
        shaderResourceViewDesc.Buffer.FirstElement = 0;
        shaderResourceViewDesc.Buffer.NumElements = ( UINT )description.Stride;
    }

    // Unordered Access View Description
    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc = {
        nativeTextureFormat,
        D3D11_UAV_DIMENSION_BUFFER
    };

    if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D ) {
        unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

        unorderedAccessViewDesc.Texture2D.MipSlice = 0;
    } else if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D ) {
        unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;

        unorderedAccessViewDesc.Texture3D.FirstWSlice = 0;
        unorderedAccessViewDesc.Texture3D.MipSlice = 0;
        unorderedAccessViewDesc.Texture3D.WSize = description.Depth;
    } else if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER
             || description.Type == BufferDesc::STRUCTURED_BUFFER
             || description.Type == BufferDesc::INDIRECT_DRAW_ARGUMENTS ) {
        unorderedAccessViewDesc.Buffer.FirstElement = 0;
        unorderedAccessViewDesc.Buffer.NumElements = ( UINT )description.Stride;
    } else if ( description.Type == BufferDesc::APPEND_STRUCTURED_BUFFER ) {
        unorderedAccessViewDesc.Buffer.FirstElement = 0;
        unorderedAccessViewDesc.Buffer.NumElements = ( UINT )description.Stride;
        unorderedAccessViewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    }

    auto nativeDevice = nativeRenderContext->nativeDevice;
    NativeBufferObject* bufferObject = new NativeBufferObject();
    if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D ) {
        TextureDescription desc;
        desc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
        desc.width = description.Width;
        desc.height = description.Height;
        desc.depth = description.Depth;
        desc.mipCount = description.MipCount;
        desc.format = description.ViewFormat;
        bufferObject->bufferTexture = flan::rendering::CreateTexture2DImpl( nativeRenderContext, desc );

        nativeDevice->CreateShaderResourceView( bufferObject->bufferTexture->textureResource, &shaderResourceViewDesc, &bufferObject->bufferResourceView );
        nativeDevice->CreateUnorderedAccessView( bufferObject->bufferTexture->textureResource, &unorderedAccessViewDesc, &bufferObject->bufferUAVObject );
    } else {
        nativeDevice->CreateBuffer( &bufferDescription, ( initialData != nullptr ) ? &subresourceDataDesc : nullptr, &bufferObject->bufferObject );
    }

    if ( description.Type == BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER 
      || description.Type == BufferDesc::STRUCTURED_BUFFER
      || description.Type == BufferDesc::APPEND_STRUCTURED_BUFFER
      || description.Type == BufferDesc::INDIRECT_DRAW_ARGUMENTS ) {
        nativeDevice->CreateShaderResourceView( bufferObject->bufferObject, &shaderResourceViewDesc, &bufferObject->bufferResourceView );
        nativeDevice->CreateUnorderedAccessView( bufferObject->bufferObject, &unorderedAccessViewDesc, &bufferObject->bufferUAVObject );
    }
      
    bufferObject->bufferType = description.Type;
    bufferObject->stride = description.Stride;

    return bufferObject;
}

void flan::rendering::DestroyBufferImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject )
{
#define D3D11_RELEASE( obj ) if ( obj != nullptr ) { obj->Release(); obj = nullptr; }

    D3D11_RELEASE( bufferObject->bufferObject );
    D3D11_RELEASE( bufferObject->bufferResourceView );
    D3D11_RELEASE( bufferObject->bufferUAVObject );
}

void flan::rendering::UpdateBufferImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize )
{
    auto nativeDeviceContext = nativeRenderContext->nativeDeviceContext;

    D3D11_MAPPED_SUBRESOURCE mappedSubResource;

    HRESULT operationResult;
    // Even if the mapping was succesful, the subresource might be null
    if ( SUCCEEDED( operationResult = nativeDeviceContext->Map( bufferObject->bufferObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource ) )
        && mappedSubResource.pData != nullptr ) {
        memset( mappedSubResource.pData, 0, dataToUploadSize );
        memcpy( mappedSubResource.pData, dataToUpload, dataToUploadSize );

        nativeDeviceContext->Unmap( bufferObject->bufferObject, 0 );
    } else {
        FLAN_CERR << "Failed to map buffer! (error code: " << operationResult << ")" << std::endl;
        return;
    }
}

void flan::rendering::UpdateBufferAsynchronousImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize )
{
    auto nativeDeviceContext = nativeCmdList->deferredContext;

    D3D11_MAPPED_SUBRESOURCE mappedSubResource;

    HRESULT operationResult;
    // Even if the mapping was succesful, the subresource might be null
    if ( SUCCEEDED( operationResult = nativeDeviceContext->Map( bufferObject->bufferObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource ) )
        && mappedSubResource.pData != nullptr ) {
        memset( mappedSubResource.pData, 0, dataToUploadSize );
        memcpy( mappedSubResource.pData, dataToUpload, dataToUploadSize );

        nativeDeviceContext->Unmap( bufferObject->bufferObject, 0 );
    } else {
        FLAN_CERR << "Failed to map buffer! (error code: " << operationResult << ")" << std::endl;
        return;
    }
}

void flan::rendering::UpdateBufferRangeImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize, const int32_t bufferOffset )
{
    auto nativeDeviceContext = nativeRenderContext->nativeDeviceContext;

    const D3D11_BOX bufferRange = { ( UINT )bufferOffset, 0U, 0U, ( UINT )( bufferOffset + dataToUploadSize ), 1U, 1U };
    nativeDeviceContext->UpdateSubresource( bufferObject->bufferObject, 0, ( bufferObject->bufferType == BufferDesc::CONSTANT_BUFFER ) ? nullptr : &bufferRange, dataToUpload, 0, 0 );
}

void flan::rendering::FlushAndUpdateBufferRangeImpl( NativeRenderContext* nativeRenderContext, NativeBufferObject* bufferObject, const void* dataToUpload, const std::size_t dataToUploadSize, const int32_t bufferOffset )
{
    auto nativeDeviceContext = nativeRenderContext->nativeDeviceContext;

    const D3D11_BOX bufferRange = { ( UINT )bufferOffset, 0U, 0U, ( UINT )( bufferOffset + dataToUploadSize ), 1U, 1U };
    nativeDeviceContext->UpdateSubresource( bufferObject->bufferObject, 0, ( bufferObject->bufferType == BufferDesc::CONSTANT_BUFFER ) ? nullptr : &bufferRange, dataToUpload, 0, 0 );
}

void flan::rendering::BindBufferCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex )
{
    auto nativeDeviceContext = nativeCmdList->deferredContext;

    switch ( bufferObject->bufferType ) {
    case BufferDesc::BufferType::CONSTANT_BUFFER:
    {
        if ( ( shaderStagesToBindTo & SHADER_STAGE_VERTEX ) == SHADER_STAGE_VERTEX ) {
            nativeDeviceContext->VSSetConstantBuffers( bindingIndex, 1, &bufferObject->bufferObject );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_PIXEL ) == SHADER_STAGE_PIXEL ) {
            nativeDeviceContext->PSSetConstantBuffers( bindingIndex, 1, &bufferObject->bufferObject );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_CONTROL ) == SHADER_STAGE_TESSELATION_CONTROL ) {
            nativeDeviceContext->DSSetConstantBuffers( bindingIndex, 1, &bufferObject->bufferObject );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_EVALUATION ) == SHADER_STAGE_TESSELATION_EVALUATION ) {
            nativeDeviceContext->HSSetConstantBuffers( bindingIndex, 1, &bufferObject->bufferObject );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_COMPUTE ) == SHADER_STAGE_COMPUTE ) {
            nativeDeviceContext->CSSetConstantBuffers( bindingIndex, 1, &bufferObject->bufferObject );
        }
    } break;
    
    case BufferDesc::BufferType::DYNAMIC_INDICE_BUFFER:
    case BufferDesc::BufferType::INDICE_BUFFER:
    {
        nativeDeviceContext->IASetIndexBuffer( bufferObject->bufferObject, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0 );
    } break;

    case BufferDesc::BufferType::DYNAMIC_VERTEX_BUFFER:
    case BufferDesc::BufferType::VERTEX_BUFFER:
    {
        constexpr UINT OFFSETS = 0;

        nativeDeviceContext->IASetVertexBuffers( bindingIndex, 1, &bufferObject->bufferObject, &bufferObject->stride, &OFFSETS );
    } break;

    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_TEXTURE_1D:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_TEXTURE_2D:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_TEXTURE_3D:
    case BufferDesc::BufferType::STRUCTURED_BUFFER:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_BUFFER:
    case BufferDesc::BufferType::INDIRECT_DRAW_ARGUMENTS:
    {
        nativeDeviceContext->CSSetUnorderedAccessViews( bindingIndex, 1, &bufferObject->bufferUAVObject, nullptr );
    } break;

    case BufferDesc::BufferType::APPEND_STRUCTURED_BUFFER:
    {
        constexpr UINT initialValueCount[1] = { 0 };
        nativeDeviceContext->CSSetUnorderedAccessViews( bindingIndex, 1, &bufferObject->bufferUAVObject, initialValueCount );
    } break;

    default:
        break;
    }
}

void flan::rendering::BindBufferReadOnlyCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex )
{
    auto nativeDeviceContext = nativeCmdList->deferredContext;

    switch ( bufferObject->bufferType ) {
    case BufferDesc::BufferType::CONSTANT_BUFFER:
    case BufferDesc::BufferType::DYNAMIC_INDICE_BUFFER:
    case BufferDesc::BufferType::INDICE_BUFFER:
    case BufferDesc::BufferType::DYNAMIC_VERTEX_BUFFER:
    case BufferDesc::BufferType::VERTEX_BUFFER:
        // No need to distinguish read/write for those kind of buffer
        BindBufferCmdImpl( nativeCmdList, bufferObject, shaderStagesToBindTo, bindingIndex );
        break;

    case BufferDesc::BufferType::INDIRECT_DRAW_ARGUMENTS:
    case BufferDesc::BufferType::STRUCTURED_BUFFER:
    case BufferDesc::BufferType::APPEND_STRUCTURED_BUFFER:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_BUFFER:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_TEXTURE_1D:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_TEXTURE_2D:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_TEXTURE_3D:
    {
        if ( ( shaderStagesToBindTo & SHADER_STAGE_VERTEX ) == SHADER_STAGE_VERTEX ) {
            nativeDeviceContext->VSSetShaderResources( bindingIndex, 1, &bufferObject->bufferResourceView );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_PIXEL ) == SHADER_STAGE_PIXEL ) {
            nativeDeviceContext->PSSetShaderResources( bindingIndex, 1, &bufferObject->bufferResourceView );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_CONTROL ) == SHADER_STAGE_TESSELATION_CONTROL ) {
            nativeDeviceContext->DSSetShaderResources( bindingIndex, 1, &bufferObject->bufferResourceView );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_EVALUATION ) == SHADER_STAGE_TESSELATION_EVALUATION ) {
            nativeDeviceContext->HSSetShaderResources( bindingIndex, 1, &bufferObject->bufferResourceView );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_COMPUTE ) == SHADER_STAGE_COMPUTE ) {
            nativeDeviceContext->CSSetShaderResources( bindingIndex, 1, &bufferObject->bufferResourceView );
        }
    } break;

    default:
        break;
    }
}

void flan::rendering::UnbindBufferCmdImpl( NativeCommandList* nativeCmdList, NativeBufferObject* bufferObject, const uint32_t shaderStagesToBindTo, const uint32_t bindingIndex, const Buffer::BindMode bindMode )
{
    auto nativeDeviceContext = nativeCmdList->deferredContext;

    constexpr ID3D11UnorderedAccessView* NO_UAV[1] = { ( ID3D11UnorderedAccessView* )nullptr };
    constexpr ID3D11ShaderResourceView* NO_SRV[1] = { ( ID3D11ShaderResourceView* )nullptr };
    constexpr ID3D11Buffer* NO_BUFFER[1] = { ( ID3D11Buffer* )nullptr };

    switch ( bufferObject->bufferType ) {
    case BufferDesc::BufferType::CONSTANT_BUFFER:
    {
        if ( ( shaderStagesToBindTo & SHADER_STAGE_VERTEX ) == SHADER_STAGE_VERTEX ) {
            nativeDeviceContext->VSSetConstantBuffers( bindingIndex, 1, NO_BUFFER );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_PIXEL ) == SHADER_STAGE_PIXEL ) {
            nativeDeviceContext->PSSetConstantBuffers( bindingIndex, 1, NO_BUFFER );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_CONTROL ) == SHADER_STAGE_TESSELATION_CONTROL ) {
            nativeDeviceContext->DSSetConstantBuffers( bindingIndex, 1, NO_BUFFER );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_EVALUATION ) == SHADER_STAGE_TESSELATION_EVALUATION ) {
            nativeDeviceContext->HSSetConstantBuffers( bindingIndex, 1, NO_BUFFER );
        }

        if ( ( shaderStagesToBindTo & SHADER_STAGE_COMPUTE ) == SHADER_STAGE_COMPUTE ) {
            nativeDeviceContext->CSSetConstantBuffers( bindingIndex, 1, NO_BUFFER );
        }
    } break;

    case BufferDesc::BufferType::DYNAMIC_INDICE_BUFFER:
    case BufferDesc::BufferType::INDICE_BUFFER:
    {
        nativeDeviceContext->IASetIndexBuffer( bufferObject->bufferObject, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0 );
    } break;

    case BufferDesc::BufferType::DYNAMIC_VERTEX_BUFFER:
    case BufferDesc::BufferType::VERTEX_BUFFER:
    {
        constexpr UINT OFFSETS = 0;

        nativeDeviceContext->IASetVertexBuffers( bindingIndex, 1, nullptr, nullptr, nullptr );
    } break;

    case BufferDesc::BufferType::STRUCTURED_BUFFER:
    case BufferDesc::BufferType::APPEND_STRUCTURED_BUFFER:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_BUFFER:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_TEXTURE_1D:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_TEXTURE_2D:
    case BufferDesc::BufferType::UNORDERED_ACCESS_VIEW_TEXTURE_3D:
    {
        if ( bindMode == Buffer::BindMode::WRITE_ONLY ) {
            nativeDeviceContext->CSSetUnorderedAccessViews( bindingIndex, 1, NO_UAV, nullptr );
        } else if ( bindMode == Buffer::BindMode::READ_ONLY ) {
            if ( ( shaderStagesToBindTo & SHADER_STAGE_VERTEX ) == SHADER_STAGE_VERTEX ) {
                nativeDeviceContext->VSSetShaderResources( bindingIndex, 1, NO_SRV );
            }

            if ( ( shaderStagesToBindTo & SHADER_STAGE_PIXEL ) == SHADER_STAGE_PIXEL ) {
                nativeDeviceContext->PSSetShaderResources( bindingIndex, 1, NO_SRV );
            }

            if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_CONTROL ) == SHADER_STAGE_TESSELATION_CONTROL ) {
                nativeDeviceContext->DSSetShaderResources( bindingIndex, 1, NO_SRV );
            }

            if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_EVALUATION ) == SHADER_STAGE_TESSELATION_EVALUATION ) {
                nativeDeviceContext->HSSetShaderResources( bindingIndex, 1, NO_SRV );
            }

            if ( ( shaderStagesToBindTo & SHADER_STAGE_COMPUTE ) == SHADER_STAGE_COMPUTE ) {
                nativeDeviceContext->CSSetShaderResources( bindingIndex, 1, NO_SRV );
            }
        }
    } break;

    default:
        break;
    }
}

void flan::rendering::CopyStructureCountImpl( NativeCommandList* nativeCmdList, NativeBufferObject* sourceBufferObject, NativeBufferObject* destinationBufferObject, const uint32_t byteOffset )
{
    nativeCmdList->deferredContext->CopyStructureCount( destinationBufferObject->bufferObject, byteOffset, sourceBufferObject->bufferUAVObject );
}
#endif
