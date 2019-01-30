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
#include <Rendering/CommandList.h>
#include "Buffer.h"

#include <Rendering/ImageFormat.h>

#include "RenderDevice.h"
#include "CommandList.h"

#include "Texture.h"

#include <d3d11.h>

Buffer* CreateConstantBuffer( ID3D11Device* device, Buffer* preallocatedBuffer, const BufferDesc& description, const void* initialData )
{
    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.size );
    bufferDescription.BindFlags |= D3D11_BIND_CONSTANT_BUFFER;
    bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDescription.Usage = D3D11_USAGE_DYNAMIC;

    device->CreateBuffer( &bufferDescription, ( initialData != nullptr ) ? &subresourceDataDesc : nullptr, &preallocatedBuffer->bufferObject );

    return preallocatedBuffer;
}

Buffer* CreateVertexBuffer( ID3D11Device* device, Buffer* preallocatedBuffer, const BufferDesc& description, const void* initialData )
{
    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.size );
    bufferDescription.BindFlags |= D3D11_BIND_VERTEX_BUFFER;

    if ( description.type == BufferDesc::DYNAMIC_VERTEX_BUFFER ) {
        bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
    } else {
        bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    }

    device->CreateBuffer( &bufferDescription, ( initialData != nullptr ) ? &subresourceDataDesc : nullptr, &preallocatedBuffer->bufferObject );

    preallocatedBuffer->bufferStride = description.stride;

    return preallocatedBuffer;
}

Buffer* CreateIndiceBuffer( ID3D11Device* device, Buffer* preallocatedBuffer, const BufferDesc& description, const void* initialData )
{
    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.size );
    bufferDescription.BindFlags |= D3D11_BIND_INDEX_BUFFER;

    if ( description.type == BufferDesc::DYNAMIC_INDICE_BUFFER ) {
        bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
    } else {
        bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    }

    device->CreateBuffer( &bufferDescription, ( initialData != nullptr ) ? &subresourceDataDesc : nullptr, &preallocatedBuffer->bufferObject );

    return preallocatedBuffer;
}

Buffer* CreateUnorderedAccessViewBuffer1D( ID3D11Device* device, Buffer* preallocatedBuffer, RenderDevice* renderDevice, const BufferDesc& description, const void* initialData )
{
    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.size );
    bufferDescription.BindFlags = ( D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS );

    bufferDescription.Usage = D3D11_USAGE_DEFAULT;

    DXGI_FORMAT viewFormat = static_cast< DXGI_FORMAT >( description.viewFormat );

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = { viewFormat, D3D11_SRV_DIMENSION_TEXTURE1D };
    shaderResourceViewDesc.Texture1D.MipLevels = -1;
    shaderResourceViewDesc.Texture1D.MostDetailedMip = 0;

    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc = {
        viewFormat,
        D3D11_UAV_DIMENSION_TEXTURE1D
    };

    unorderedAccessViewDesc.Texture1D.MipSlice = 0;

    TextureDescription desc;
    desc.dimension = TextureDescription::DIMENSION_TEXTURE_1D;
    desc.width = description.width;
    desc.height = description.height;
    desc.depth = description.depth;
    desc.mipCount = description.mipCount;
    desc.format = description.viewFormat;

    preallocatedBuffer->bufferTexture = renderDevice->createTexture1D( desc, initialData );
    device->CreateShaderResourceView( preallocatedBuffer->bufferObject, &shaderResourceViewDesc, &preallocatedBuffer->bufferResourceView );
    device->CreateUnorderedAccessView( preallocatedBuffer->bufferObject, &unorderedAccessViewDesc, &preallocatedBuffer->bufferUAVObject );

    return preallocatedBuffer;
}

Buffer* CreateUnorderedAccessViewBuffer2D( ID3D11Device* device, Buffer* preallocatedBuffer, RenderDevice* renderDevice, const BufferDesc& description, const void* initialData )
{
    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.size );
    bufferDescription.BindFlags = ( D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS );

    bufferDescription.Usage = D3D11_USAGE_DEFAULT;

    DXGI_FORMAT viewFormat = static_cast< DXGI_FORMAT >( description.viewFormat );

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = { viewFormat, D3D11_SRV_DIMENSION_TEXTURE2D };
    shaderResourceViewDesc.Texture2D.MipLevels = -1;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc = { 
        viewFormat,
        D3D11_UAV_DIMENSION_TEXTURE2D 
    };

    unorderedAccessViewDesc.Texture2D.MipSlice = 0;

    TextureDescription desc;
    desc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    desc.width = description.width;
    desc.height = description.height;
    desc.depth = description.depth;
    desc.mipCount = description.mipCount;
    desc.format = description.viewFormat;

    preallocatedBuffer->bufferTexture = renderDevice->createTexture2D( desc, initialData );
    device->CreateShaderResourceView( preallocatedBuffer->bufferObject, &shaderResourceViewDesc, &preallocatedBuffer->bufferResourceView );
    device->CreateUnorderedAccessView( preallocatedBuffer->bufferObject, &unorderedAccessViewDesc, &preallocatedBuffer->bufferUAVObject );

    return preallocatedBuffer;
}

Buffer* CreateUnorderedAccessViewBuffer3D( ID3D11Device* device, Buffer* preallocatedBuffer, RenderDevice* renderDevice, const BufferDesc& description, const void* initialData )
{
    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.size );
    bufferDescription.BindFlags = ( D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS );

    bufferDescription.Usage = D3D11_USAGE_DEFAULT;

    DXGI_FORMAT viewFormat = static_cast< DXGI_FORMAT >( description.viewFormat );

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = { viewFormat, D3D11_SRV_DIMENSION_TEXTURE3D };
    shaderResourceViewDesc.Texture3D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture3D.MipLevels = -1;

    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc = {
        viewFormat,
        D3D11_UAV_DIMENSION_TEXTURE3D
    };

    unorderedAccessViewDesc.Texture3D.FirstWSlice = 0;
    unorderedAccessViewDesc.Texture3D.MipSlice = 0;
    unorderedAccessViewDesc.Texture3D.WSize = description.depth;

    TextureDescription desc;
    desc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    desc.width = description.width;
    desc.height = description.height;
    desc.depth = description.depth;
    desc.mipCount = description.mipCount;
    desc.format = description.viewFormat;

    preallocatedBuffer->bufferTexture = renderDevice->createTexture3D( desc, initialData );
    device->CreateShaderResourceView( preallocatedBuffer->bufferObject, &shaderResourceViewDesc, &preallocatedBuffer->bufferResourceView );
    device->CreateUnorderedAccessView( preallocatedBuffer->bufferObject, &unorderedAccessViewDesc, &preallocatedBuffer->bufferUAVObject );

    return preallocatedBuffer;
}

Buffer* CreateStructuredBuffer( ID3D11Device* device, Buffer* preallocatedBuffer, const BufferDesc& description, const void* initialData )
{
    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    // Buffer
    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.size );
    bufferDescription.BindFlags = ( D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS );
    bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    bufferDescription.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDescription.StructureByteStride = static_cast<UINT>( description.size );
    bufferDescription.ByteWidth *= static_cast<UINT>( description.stride );

    // SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = { 
        DXGI_FORMAT_UNKNOWN,
        D3D11_SRV_DIMENSION_BUFFER 
    };
    shaderResourceViewDesc.Buffer.FirstElement = 0;
    shaderResourceViewDesc.Buffer.ElementWidth = ( UINT )description.stride;

    // UAV
    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc = { 
        DXGI_FORMAT_UNKNOWN,
        D3D11_UAV_DIMENSION_BUFFER 
    };
    unorderedAccessViewDesc.Buffer.FirstElement = 0;
    unorderedAccessViewDesc.Buffer.NumElements = ( UINT )description.stride;

    if ( description.type == BufferDesc::APPEND_STRUCTURED_BUFFER ) {
        unorderedAccessViewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    }

    device->CreateBuffer( &bufferDescription, ( initialData != nullptr ) ? &subresourceDataDesc : nullptr, &preallocatedBuffer->bufferObject );
    device->CreateShaderResourceView( preallocatedBuffer->bufferObject, &shaderResourceViewDesc, &preallocatedBuffer->bufferResourceView );
    device->CreateUnorderedAccessView( preallocatedBuffer->bufferObject, &unorderedAccessViewDesc, &preallocatedBuffer->bufferUAVObject );

    return preallocatedBuffer;
}

Buffer* CreateIndirectDrawArgsBuffer( ID3D11Device* device, Buffer* preallocatedBuffer, const BufferDesc& description, const void* initialData )
{
    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    // Buffer
    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.size );
    bufferDescription.BindFlags = ( D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS );
    bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    bufferDescription.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
    bufferDescription.StructureByteStride = static_cast<UINT>( description.size );
    bufferDescription.ByteWidth *= static_cast<UINT>( description.stride );

    DXGI_FORMAT viewFormat = static_cast< DXGI_FORMAT >( description.viewFormat );

    // SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
        viewFormat,
        D3D11_SRV_DIMENSION_BUFFER
    };
    shaderResourceViewDesc.Buffer.FirstElement = 0;
    shaderResourceViewDesc.Buffer.ElementWidth = ( UINT )description.stride;

    // UAV
    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc = {
        viewFormat,
        D3D11_UAV_DIMENSION_BUFFER
    };
    unorderedAccessViewDesc.Buffer.FirstElement = 0;
    unorderedAccessViewDesc.Buffer.NumElements = ( UINT )description.stride;

    device->CreateBuffer( &bufferDescription, ( initialData != nullptr ) ? &subresourceDataDesc : nullptr, &preallocatedBuffer->bufferObject );
    device->CreateShaderResourceView( preallocatedBuffer->bufferObject, &shaderResourceViewDesc, &preallocatedBuffer->bufferResourceView );
    device->CreateUnorderedAccessView( preallocatedBuffer->bufferObject, &unorderedAccessViewDesc, &preallocatedBuffer->bufferUAVObject );

    return preallocatedBuffer;
}

Buffer* CreateUAVBuffer( ID3D11Device* device, Buffer* preallocatedBuffer, const BufferDesc& description, const void* initialData )
{
    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    // Buffer
    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.size );
    bufferDescription.BindFlags = ( D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS );
    bufferDescription.Usage = D3D11_USAGE_DEFAULT;

    DXGI_FORMAT viewFormat = static_cast< DXGI_FORMAT >( description.viewFormat );

    // SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
        viewFormat,
        D3D11_SRV_DIMENSION_BUFFER
    };
    shaderResourceViewDesc.Buffer.FirstElement = 0;
    shaderResourceViewDesc.Buffer.ElementWidth = ( UINT )description.stride;

    // UAV
    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc = {
        viewFormat,
        D3D11_UAV_DIMENSION_BUFFER
    };
    unorderedAccessViewDesc.Buffer.FirstElement = 0;
    unorderedAccessViewDesc.Buffer.NumElements = ( UINT )description.stride;

    device->CreateBuffer( &bufferDescription, ( initialData != nullptr ) ? &subresourceDataDesc : nullptr, &preallocatedBuffer->bufferObject );
    device->CreateShaderResourceView( preallocatedBuffer->bufferObject, &shaderResourceViewDesc, &preallocatedBuffer->bufferResourceView );
    device->CreateUnorderedAccessView( preallocatedBuffer->bufferObject, &unorderedAccessViewDesc, &preallocatedBuffer->bufferUAVObject );

    return preallocatedBuffer;
}

Buffer* CreateGenericBuffer( ID3D11Device* device, Buffer* preallocatedBuffer, const BufferDesc& description, const void* initialData )
{
    // Subresource description
    D3D11_SUBRESOURCE_DATA subresourceDataDesc = {};
    subresourceDataDesc.pSysMem = initialData;

    // Buffer
    D3D11_BUFFER_DESC bufferDescription = {};
    bufferDescription.ByteWidth = static_cast<UINT>( description.size );
    bufferDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDescription.Usage = D3D11_USAGE_DYNAMIC;

    DXGI_FORMAT viewFormat = static_cast< DXGI_FORMAT >( description.viewFormat );

    // SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
        viewFormat,
        D3D11_SRV_DIMENSION_BUFFER
    };
    shaderResourceViewDesc.Buffer.FirstElement = 0;
    shaderResourceViewDesc.Buffer.ElementWidth = ( UINT )description.stride;

    device->CreateBuffer( &bufferDescription, ( initialData != nullptr ) ? &subresourceDataDesc : nullptr, &preallocatedBuffer->bufferObject );
    device->CreateShaderResourceView( preallocatedBuffer->bufferObject, &shaderResourceViewDesc, &preallocatedBuffer->bufferResourceView );

    return preallocatedBuffer;
}

Buffer* RenderDevice::createBuffer( const BufferDesc& description, const void* initialData )
{
    ID3D11Device* nativeDevice = renderContext->nativeDevice;
    Buffer* buffer = nya::core::allocate<Buffer>( memoryAllocator );

    switch ( description.type ) {
    case BufferDesc::CONSTANT_BUFFER:
        return CreateConstantBuffer( nativeDevice, buffer, description, initialData );

    case BufferDesc::VERTEX_BUFFER:
    case BufferDesc::DYNAMIC_VERTEX_BUFFER:
        return CreateVertexBuffer( nativeDevice, buffer, description, initialData );

    case BufferDesc::INDICE_BUFFER:
    case BufferDesc::DYNAMIC_INDICE_BUFFER:
        return CreateIndiceBuffer( nativeDevice, buffer, description, initialData );

    case BufferDesc::APPEND_STRUCTURED_BUFFER:
    case BufferDesc::STRUCTURED_BUFFER:
        return CreateStructuredBuffer( nativeDevice, buffer, description, initialData );

    case BufferDesc::INDIRECT_DRAW_ARGUMENTS:
        return CreateIndirectDrawArgsBuffer( nativeDevice, buffer, description, initialData );

    case BufferDesc::UNORDERED_ACCESS_VIEW_BUFFER:
        return CreateUAVBuffer( nativeDevice, buffer, description, initialData );

    case BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_1D:
        return CreateUnorderedAccessViewBuffer1D( nativeDevice, buffer, this, description, initialData );

    case BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_2D:
        return CreateUnorderedAccessViewBuffer2D( nativeDevice, buffer, this, description, initialData );

    case BufferDesc::UNORDERED_ACCESS_VIEW_TEXTURE_3D:
        return CreateUnorderedAccessViewBuffer3D( nativeDevice, buffer, this, description, initialData );

    case BufferDesc::GENERIC_BUFFER:
        return CreateGenericBuffer( nativeDevice, buffer, description, initialData );

    default:
        break;
    }

    NYA_ASSERT( false, "Invalid/Unsupported buffer type provided!" );

    return nullptr;
}

void RenderDevice::destroyBuffer( Buffer* buffer )
{
    buffer->bufferObject->Release();

#define RELEASE_IF_ALLOCATED( obj ) if ( obj != nullptr ) { obj->Release(); }

    RELEASE_IF_ALLOCATED( buffer->bufferResourceView );
    RELEASE_IF_ALLOCATED( buffer->bufferUAVObject );

    nya::core::free( memoryAllocator, buffer );
}

void RenderDevice::setDebugMarker( Buffer* buffer, const char* objectName )
{
    buffer->bufferObject->SetPrivateData( WKPDID_D3DDebugObjectName, static_cast< UINT >( strlen( objectName ) ), objectName );
}

void CommandList::bindVertexBuffer( const Buffer* buffer, const unsigned int bindIndex )
{
    constexpr UINT OFFSETS = 0;

    NativeCommandList->deferredContext->IASetVertexBuffers( bindIndex, 1, &buffer->bufferObject, &buffer->bufferStride, &OFFSETS );
}

void CommandList::bindIndiceBuffer( const Buffer* buffer )
{
    NativeCommandList->deferredContext->IASetIndexBuffer( buffer->bufferObject, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0u );
}

void CommandList::updateBuffer( Buffer* buffer, const void* data, const size_t dataSize )
{
    ID3D11DeviceContext* nativeDeviceContext = NativeCommandList->deferredContext;

    D3D11_MAPPED_SUBRESOURCE mappedSubResource;
    HRESULT operationResult = nativeDeviceContext->Map( buffer->bufferObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource );

    if ( SUCCEEDED( operationResult ) ) {
        memset( mappedSubResource.pData, 0, dataSize );
        memcpy( mappedSubResource.pData, data, dataSize );

        nativeDeviceContext->Unmap( buffer->bufferObject, 0 );
    } else {
        NYA_CERR << "Failed to map buffer! (error code: " << NYA_PRINT_HEX( operationResult ) << ")" << std::endl;
    }
}

void CommandList::copyStructureCount( Buffer* srcBuffer, Buffer* dstBuffer, const unsigned int offset )
{
    NativeCommandList->deferredContext->CopyStructureCount( dstBuffer->bufferObject, offset, srcBuffer->bufferUAVObject );
}

void CommandList::drawInstancedIndirect( const Buffer* drawArgsBuffer, const unsigned int bufferDataOffset )
{
    NativeCommandList->deferredContext->DrawInstancedIndirect( drawArgsBuffer->bufferObject, bufferDataOffset );
}
#endif
