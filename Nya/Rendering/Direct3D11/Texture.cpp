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

#include <Rendering/ImageFormat.h>

#include "RenderDevice.h"
#include "CommandList.h"

#include "Texture.h"

#include <Maths/Helpers.h>

#include <d3d11.h>
#include <vector>

bool IsUsingCompressedFormat( const eImageFormat format )
{
    return format == IMAGE_FORMAT_BC1_TYPELESS
        || format == IMAGE_FORMAT_BC1_UNORM
        || format == IMAGE_FORMAT_BC1_UNORM_SRGB
        || format == IMAGE_FORMAT_BC2_TYPELESS
        || format == IMAGE_FORMAT_BC2_UNORM
        || format == IMAGE_FORMAT_BC2_UNORM_SRGB
        || format == IMAGE_FORMAT_BC3_TYPELESS
        || format == IMAGE_FORMAT_BC3_UNORM
        || format == IMAGE_FORMAT_BC3_UNORM_SRGB
        || format == IMAGE_FORMAT_BC4_TYPELESS
        || format == IMAGE_FORMAT_BC4_UNORM
        || format == IMAGE_FORMAT_BC4_SNORM
        || format == IMAGE_FORMAT_BC5_TYPELESS
        || format == IMAGE_FORMAT_BC5_UNORM
        || format == IMAGE_FORMAT_BC5_SNORM;
}

UINT GetBindFlags( const TextureDescription& description )
{
    // Generate bind flag set
    UINT bindFlags = D3D11_BIND_SHADER_RESOURCE;

    if ( description.flags.isDepthResource ) {
        bindFlags |= D3D11_BIND_DEPTH_STENCIL;
    } else if ( !IsUsingCompressedFormat( description.format ) && description.flags.allowCPUWrite == 0 ) {
        // Safety check in case the user try to use a compressed format as a rendertarget
        bindFlags |= D3D11_BIND_RENDER_TARGET;

        if ( description.samplerCount <= 1 ) {
            bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
        }
    }

    return bindFlags;
}

UINT GetMiscFlags( const TextureDescription& description )
{
    // Generate miscellaneous flag set
    UINT miscFlags = 0;
    if ( description.flags.isCubeMap ) {
        miscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    }

    if ( description.flags.useHardwareMipGen ) {
        miscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    return miscFlags;
}

size_t BitsPerPixel( _In_ DXGI_FORMAT fmt )
{
    switch ( fmt ) {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return 128;

    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return 96;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_TYPELESS:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    case DXGI_FORMAT_Y416:
    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
        return 64;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R16G16_TYPELESS:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DXGI_FORMAT_AYUV:
    case DXGI_FORMAT_Y410:
    case DXGI_FORMAT_YUY2:
        return 32;

    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
        return 24;

    case DXGI_FORMAT_R8G8_TYPELESS:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_A8P8:
    case DXGI_FORMAT_B4G4R4A4_UNORM:
        return 16;

    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_420_OPAQUE:
    case DXGI_FORMAT_NV11:
        return 12;

    case DXGI_FORMAT_R8_TYPELESS:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_AI44:
    case DXGI_FORMAT_IA44:
    case DXGI_FORMAT_P8:
        return 8;

    case DXGI_FORMAT_R1_UNORM:
        return 1;

    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        return 4;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return 8;

    default:
        return 0;
    }
}

void GetSurfaceInfo( size_t width, size_t height, DXGI_FORMAT fmt, size_t* outNumBytes, size_t* outRowBytes, size_t* outNumRows )
{
    size_t numBytes = 0;
    size_t rowBytes = 0;
    size_t numRows = 0;

    bool bc = false;
    bool packed = false;
    bool planar = false;
    size_t bpe = 0;
    switch ( fmt ) {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        bc = true;
        bpe = 8;
        break;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        bc = true;
        bpe = 16;
        break;

    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_YUY2:
        packed = true;
        bpe = 4;
        break;

    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
        packed = true;
        bpe = 8;
        break;

    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_420_OPAQUE:
        planar = true;
        bpe = 2;
        break;

    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
        planar = true;
        bpe = 4;
        break;
    }

    if ( bc ) {
        size_t numBlocksWide = 0;
        if ( width > 0 ) {
            numBlocksWide = nya::maths::max( 1, ( width + 3 ) / 4 );
        }
        size_t numBlocksHigh = 0;
        if ( height > 0 ) {
            numBlocksHigh = nya::maths::max( 1, ( height + 3 ) / 4 );
        }
        rowBytes = numBlocksWide * bpe;
        numRows = numBlocksHigh;
        numBytes = rowBytes * numBlocksHigh;
    } else if ( packed ) {
        rowBytes = ( ( width + 1 ) >> 1 ) * bpe;
        numRows = height;
        numBytes = rowBytes * height;
    } else if ( fmt == DXGI_FORMAT_NV11 ) {
        rowBytes = ( ( width + 3 ) >> 2 ) * 4;
        numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
        numBytes = rowBytes * numRows;
    } else if ( planar ) {
        rowBytes = ( ( width + 1 ) >> 1 ) * bpe;
        numBytes = ( rowBytes * height ) + ( ( rowBytes * height + 1 ) >> 1 );
        numRows = height + ( ( height + 1 ) >> 1 );
    } else {
        size_t bpp = BitsPerPixel( fmt );
        rowBytes = ( width * bpp + 7 ) / 8; // round up to nearest byte
        numRows = height;
        numBytes = rowBytes * height;
    }

    if ( outNumBytes ) {
        *outNumBytes = numBytes;
    }
    if ( outRowBytes ) {
        *outRowBytes = rowBytes;
    }
    if ( outNumRows ) {
        *outNumRows = numRows;
    }
}

void GetSubResourceDescriptor( std::vector<D3D11_SUBRESOURCE_DATA>& subResourceData, const void* initialData, const TextureDescription& description, const DXGI_FORMAT nativeTextureFormat )
{
    size_t NumBytes = 0;
    size_t RowBytes = 0;
    size_t index = 0;

    uint8_t* srcBits = ( uint8_t* )initialData;
    for ( unsigned int i = 0; i < description.arraySize; i++ ) {
        size_t w = description.width;
        size_t h = description.height;
        size_t d = description.depth;
        for ( unsigned int j = 0; j < description.mipCount; j++ ) {
            GetSurfaceInfo( w,
                h,
                nativeTextureFormat,
                &NumBytes,
                &RowBytes,
                nullptr
            );

            subResourceData[index].pSysMem = ( const void* )srcBits;
            subResourceData[index].SysMemPitch = static_cast<UINT>( RowBytes );
            subResourceData[index].SysMemSlicePitch = static_cast<UINT>( NumBytes );
            ++index;

            srcBits += NumBytes * d;

            w = w >> 1;
            h = h >> 1;
            d = d >> 1;

            if ( w == 0 ) {
                w = 1;
            }
            if ( h == 0 ) {
                h = 1;
            }
            if ( d == 0 ) {
                d = 1;
            }
        }
    }
}

Texture* RenderDevice::createTexture1D( const TextureDescription& description, const void* initialData, const size_t initialDataSize )
{
    // Generate bind flag set
    UINT bindFlags = GetBindFlags( description );

    // Generate miscellaneous flag set
    UINT miscFlags = GetMiscFlags( description );

    const bool useMultisamplePattern = description.flags.useMultisamplePattern == 1;
    const UINT sampleDescQuality = ( useMultisamplePattern ) ? D3D11_STANDARD_MULTISAMPLE_PATTERN : static_cast<UINT>( 0 );

    // Create native texture description object
    DXGI_FORMAT nativeTextureFormat = static_cast< DXGI_FORMAT >( description.format );
    const D3D11_TEXTURE1D_DESC renderTargetDesc = {
        description.width,				// UINT Width
        description.mipCount,			// UINT MipLevels
        description.arraySize,
        nativeTextureFormat,	        // DXGI_FORMAT Format
        ( description.flags.allowCPUWrite == 1 ) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,			// D3D11_USAGE Usage
        bindFlags,		                // UINT BindFlags
        ( description.flags.allowCPUWrite == 1 ) ? D3D11_CPU_ACCESS_WRITE : static_cast<UINT>( 0 ),			// UINT CPUAccessFlags
        miscFlags,  					// UINT MiscFlags
    };

    // Create Texture SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
        nativeTextureFormat,
        D3D11_SRV_DIMENSION_TEXTURE1D,
    };

    // Build SRV description
    shaderResourceViewDesc.Texture1D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture1D.MipLevels = ( description.mipCount <= 0 ) ? -1 : description.mipCount;

    std::vector<D3D11_SUBRESOURCE_DATA> subResourceData( nya::maths::max( 1u, description.mipCount ) * description.arraySize );
    GetSubResourceDescriptor( subResourceData, initialData, description, nativeTextureFormat );

    Texture* texture = nya::core::allocate<Texture>( memoryAllocator );

    // Create resource first
    ID3D11Device* device = renderContext->nativeDevice;

    HRESULT operationResult = device->CreateTexture1D( &renderTargetDesc, ( initialData != nullptr ) ? subResourceData.data() : nullptr, &texture->texture1D );
    
    NYA_ASSERT( SUCCEEDED( operationResult ), "Texture creation failed! (error code 0x%X)", operationResult );

    // Create resource view linked to the resource created previously
    device->CreateShaderResourceView( texture->texture1D, &shaderResourceViewDesc, &texture->shaderResourceView );

    return texture;
}

Texture* RenderDevice::createTexture2D( const TextureDescription& description, const void* initialData, const size_t initialDataSize )
{
    // Generate bind flag set
    UINT bindFlags = GetBindFlags( description );

    // Generate miscellaneous flag set
    UINT miscFlags = GetMiscFlags( description );

    const bool useMultisamplePattern = description.flags.useMultisamplePattern == 1;
    const UINT sampleDescQuality = ( useMultisamplePattern ) ? D3D11_STANDARD_MULTISAMPLE_PATTERN : static_cast<UINT>( 0 );

    // Create native texture description object
    DXGI_FORMAT nativeTextureFormat = static_cast< DXGI_FORMAT >( description.format );
    const D3D11_TEXTURE2D_DESC renderTargetDesc = {
        description.width,				// UINT Width
        description.height,				// UINT Height
        description.mipCount,			// UINT MipLevels
        description.arraySize,			// UINT ArraySize
        nativeTextureFormat,	        // DXGI_FORMAT Format
        {								// DXGI_SAMPLE_DESC SampleDesc
            description.samplerCount,	//		UINT Count
            sampleDescQuality, 	        //		UINT Quality
        },								//
        ( description.flags.allowCPUWrite == 1 ) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,			// D3D11_USAGE Usage
        bindFlags,		                // UINT BindFlags
        ( description.flags.allowCPUWrite == 1 ) ? D3D11_CPU_ACCESS_WRITE : static_cast<UINT>( 0 ),			// UINT CPUAccessFlags
        miscFlags,  					// UINT MiscFlags
    };

    // Create Texture SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
        nativeTextureFormat,
    };

    switch ( nativeTextureFormat ) {
    case DXGI_FORMAT_R32_TYPELESS:
        shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
        break;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        break;
    }

    // Build SRV description
    if ( description.samplerCount <= 1 ) {
        if ( description.flags.isCubeMap ) {
            if ( description.arraySize > 1 ) {
                shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;

                shaderResourceViewDesc.TextureCubeArray.MostDetailedMip = 0;
                shaderResourceViewDesc.TextureCubeArray.MipLevels = description.mipCount <= 0 ? -1 : description.mipCount;
                shaderResourceViewDesc.TextureCubeArray.First2DArrayFace = 0;
                shaderResourceViewDesc.TextureCubeArray.NumCubes = ( UINT )description.arraySize / description.depth;

            } else {
                shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

                shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
                shaderResourceViewDesc.TextureCube.MipLevels = description.mipCount <= 0 ? -1 : description.mipCount;
            }
        } else if ( description.arraySize > 1 ) {
            shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;

            shaderResourceViewDesc.Texture2DArray.ArraySize = description.arraySize;
            shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
            shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
            shaderResourceViewDesc.Texture2DArray.MipLevels = description.mipCount <= 1 ? -1 : description.mipCount;
        } else {
            shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

            shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
            shaderResourceViewDesc.Texture2D.MipLevels = description.mipCount <= 1 ? -1 : description.mipCount;
        }
    } else {
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
    }

    std::vector<D3D11_SUBRESOURCE_DATA> subResourceData( nya::maths::max( 1u, description.mipCount ) * description.arraySize );
    GetSubResourceDescriptor( subResourceData, initialData, description, nativeTextureFormat );

    Texture* texture = nya::core::allocate<Texture>( memoryAllocator );

    // Create resource first
    ID3D11Device* device = renderContext->nativeDevice;

    HRESULT operationResult = device->CreateTexture2D( &renderTargetDesc, ( initialData != nullptr ) ? subResourceData.data() : nullptr, &texture->texture2D );

    if ( !SUCCEEDED( operationResult ) )
        NYA_TRIGGER_BREAKPOINT

    NYA_ASSERT( SUCCEEDED( operationResult ), "Texture creation failed! (error code 0x%X)", operationResult );

    // Create resource view linked to the resource created previously
    device->CreateShaderResourceView( texture->texture2D, &shaderResourceViewDesc, &texture->shaderResourceView );

    return texture;
}

Texture* RenderDevice::createTexture3D( const TextureDescription& description, const void* initialData, const size_t initialDataSize )
{
    // Generate bind flag set
    UINT bindFlags = GetBindFlags( description );

    // Generate miscellaneous flag set
    UINT miscFlags = GetMiscFlags( description );

    const bool useMultisamplePattern = description.flags.useMultisamplePattern == 1;
    const UINT sampleDescQuality = ( useMultisamplePattern ) ? D3D11_STANDARD_MULTISAMPLE_PATTERN : static_cast<UINT>( 0 );
    
    // Create native texture description object
    DXGI_FORMAT nativeTextureFormat = static_cast< DXGI_FORMAT >( description.format );
    const D3D11_TEXTURE3D_DESC renderTargetDesc = {
        description.width,				// UINT Width
        description.height,				// UINT Height
        description.depth,			    // UINT ArraySize
        description.mipCount,			// UINT MipLevels
        nativeTextureFormat,	        // DXGI_FORMAT Format
        ( description.flags.allowCPUWrite == 1 ) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,			// D3D11_USAGE Usage
        bindFlags,		                // UINT BindFlags
        ( description.flags.allowCPUWrite == 1 ) ? D3D11_CPU_ACCESS_WRITE : static_cast<UINT>( 0 ),			// UINT CPUAccessFlags
        miscFlags,  					// UINT MiscFlags
    };

    // Create Texture SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
        nativeTextureFormat,
        D3D11_SRV_DIMENSION_TEXTURE3D,
    };

    // Build SRV description
    shaderResourceViewDesc.Texture3D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture3D.MipLevels = ( description.mipCount <= 0 ) ? -1 : description.mipCount;

    std::vector<D3D11_SUBRESOURCE_DATA> subResourceData( nya::maths::max( 1u, description.mipCount ) * description.arraySize );
    GetSubResourceDescriptor( subResourceData, initialData, description, nativeTextureFormat );

    Texture* texture = nya::core::allocate<Texture>( memoryAllocator );

    // Create resource first
    ID3D11Device* device = renderContext->nativeDevice;

    HRESULT operationResult = device->CreateTexture3D( &renderTargetDesc, ( initialData != nullptr ) ? subResourceData.data() : nullptr, &texture->texture3D );

    NYA_ASSERT( SUCCEEDED( operationResult ), "Texture creation failed! (error code 0x%X)", operationResult );

    // Create resource view linked to the resource created previously
    device->CreateShaderResourceView( texture->texture3D, &shaderResourceViewDesc, &texture->shaderResourceView );

    return texture;
}

void RenderDevice::destroyTexture( Texture* texture )
{
    texture->textureResource->Release();
    texture->shaderResourceView->Release();

    nya::core::free( memoryAllocator, texture );
}

void RenderDevice::setDebugMarker( Texture* texture, const char* objectName )
{
    texture->textureResource->SetPrivateData( WKPDID_D3DDebugObjectName, static_cast< UINT >( strlen( objectName ) ), objectName );
}

void CommandList::updateTexture3D( Texture* texture, const void* data, const size_t texelSize, const size_t width, const size_t height, const size_t depth )
{
    ID3D11DeviceContext* nativeDeviceContext = NativeCommandList->deferredContext;

    D3D11_MAPPED_SUBRESOURCE mappedSubResource;
    HRESULT operationResult = nativeDeviceContext->Map( texture->textureResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource );

    if ( SUCCEEDED( operationResult ) ) {
        for ( uint32_t z = 0; z < depth; z++ ) {
            const uint8_t* depthTexelsPtr = ( uint8_t* )data + ( z * ( height * width ) ) * texelSize;

            for ( uint32_t y = 0; y < height; y++ ) {
                const void* pitchTexelsPtr = depthTexelsPtr + y * width * texelSize;

                memcpy( ( ( uint8_t* )mappedSubResource.pData ) + z * mappedSubResource.DepthPitch + y * mappedSubResource.RowPitch, pitchTexelsPtr, ( width * texelSize ) );
            }
        }

        nativeDeviceContext->Unmap( texture->textureResource, 0 );
    } else {
        NYA_CERR << "Failed to map 3D texture! (error code: " << NYA_PRINT_HEX( operationResult ) << ")" << std::endl;
    }
}
#endif
