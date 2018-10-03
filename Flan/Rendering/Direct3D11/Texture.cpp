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
#include "Texture.h"
#include "RenderContext.h"
#include "CommandList.h"

#include <d3d11.h>
#include <vector>

ID3D11Texture2D* RetrieveStagingTexture( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description )
{
    ID3D11Texture2D* stagingTex = nullptr;

    auto nativeDevice = nativeRenderContext->nativeDevice;
    auto nativeDeviceContext = nativeRenderContext->nativeDeviceContext;

    D3D11_TEXTURE2D_DESC texDesc;
    textureObject->texture2D->GetDesc( &texDesc );

    if ( description.samplerCount > 1 ) {
        // MSAA content must be resolved before being copied to a staging texture
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;

        ID3D11Texture2D* tmpTex = nullptr;
        auto hr = nativeDevice->CreateTexture2D( &texDesc, 0, &tmpTex );
        if ( FAILED( hr ) ) {
            return nullptr;
        }

        DXGI_FORMAT fmt = texDesc.Format;

        UINT support = 0;
        hr = nativeDevice->CheckFormatSupport( fmt, &support );
        if ( FAILED( hr ) ) {
            return nullptr;
        }

        if ( !( support & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE ) ) {
            return nullptr;
        }

        for ( UINT item = 0; item < texDesc.ArraySize; ++item ) {
            for ( UINT level = 0; level < texDesc.MipLevels; ++level ) {
                UINT index = D3D11CalcSubresource( level, item, texDesc.MipLevels );
                nativeDeviceContext->ResolveSubresource( tmpTex, index, textureObject->textureResource, index, fmt );
            }
        }

        texDesc.BindFlags = 0;
        texDesc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
        texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        texDesc.Usage = D3D11_USAGE_STAGING;

        hr = nativeDevice->CreateTexture2D( &texDesc, 0, &stagingTex );
        if ( FAILED( hr ) ) {
            return nullptr;
        }

        nativeDeviceContext->CopyResource( stagingTex, tmpTex );

        tmpTex->Release();
    } else {
        // Otherwise, create a staging texture from the non-MSAA source
        texDesc.BindFlags = 0;
        texDesc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
        texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        texDesc.Usage = D3D11_USAGE_STAGING;

        auto hr = nativeDevice->CreateTexture2D( &texDesc, 0, &stagingTex );
        if ( FAILED( hr ) ) {
            return nullptr;
        }

        nativeDeviceContext->CopyResource( stagingTex, textureObject->textureResource );
    }

    return stagingTex;
}

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

// Stolen from Microsoft DDSTextureLoader 
static size_t BitsPerPixel( _In_ DXGI_FORMAT fmt )
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

void GetSurfaceInfo(
    _In_ size_t width,
    _In_ size_t height,
    _In_ DXGI_FORMAT fmt,
    size_t* outNumBytes,
    _Out_opt_ size_t* outRowBytes,
    _Out_opt_ size_t* outNumRows )
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
            numBlocksWide = std::max<size_t>( 1, ( width + 3 ) / 4 );
        }
        size_t numBlocksHigh = 0;
        if ( height > 0 ) {
            numBlocksHigh = std::max<size_t>( 1, ( height + 3 ) / 4 );
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

UINT GetBindFlags( const TextureDescription& description )
{
    // Generate bind flag set
    UINT bindFlags = D3D11_BIND_SHADER_RESOURCE;

    if ( description.flags.isDepthResource ) {
        bindFlags |= D3D11_BIND_DEPTH_STENCIL;
    } else if ( !IsUsingCompressedFormat( description.format ) ) {
        // Toggle renderable flags if the texture is not using a compressed format (dds, ktx, etc.)
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

void GetSubResourceDescriptor( std::vector<D3D11_SUBRESOURCE_DATA>& subResourceData, void* initialData, const TextureDescription& description, const DXGI_FORMAT nativeTextureFormat )
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

void UpdateNativeTextureDescription( const TextureDescription& description, NativeTextureObject* textureObject )
{
    textureObject->textureWidth = description.width;
    textureObject->textureHeight = description.height;
    textureObject->textureMipCount = description.mipCount;
    textureObject->textureArraySize = description.arraySize;
}

NativeTextureObject* flan::rendering::CreateTexture1DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
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
        D3D11_USAGE_DEFAULT,			// D3D11_USAGE Usage
        bindFlags,		                // UINT BindFlags
        static_cast<UINT>( 0 ),			// UINT CPUAccessFlags
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

    std::vector<D3D11_SUBRESOURCE_DATA> subResourceData( std::max( 1u, description.mipCount ) * description.arraySize );
    GetSubResourceDescriptor( subResourceData, initialData, description, nativeTextureFormat );

    NativeTextureObject* textureObject = new NativeTextureObject();
    auto nativeDevice = nativeRenderContext->nativeDevice;
    auto operationResult = nativeDevice->CreateTexture1D( &renderTargetDesc, initialData != nullptr ? subResourceData.data() : nullptr, &textureObject->texture1D );

    if ( FAILED( operationResult ) ) {
        FLAN_CERR << "Texture creation failed! (error code " << operationResult << ")" << std::endl;
        return nullptr;
    }

    nativeDevice->CreateShaderResourceView( textureObject->texture1D, &shaderResourceViewDesc, &textureObject->textureShaderResourceView );

    UpdateNativeTextureDescription( description, textureObject );

    return textureObject;
}

NativeTextureObject* flan::rendering::CreateTexture2DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
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
        D3D11_USAGE_DEFAULT,			// D3D11_USAGE Usage
        bindFlags,		                // UINT BindFlags
        static_cast<UINT>( 0 ),			// UINT CPUAccessFlags
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

    std::vector<D3D11_SUBRESOURCE_DATA> subResourceData( std::max( 1u, description.mipCount ) * description.arraySize );
    GetSubResourceDescriptor( subResourceData, initialData, description, nativeTextureFormat );

    NativeTextureObject* nativeTextureObject = new NativeTextureObject();
    auto nativeDevice = nativeRenderContext->nativeDevice;
    auto operationResult = nativeDevice->CreateTexture2D( &renderTargetDesc, ( initialData != nullptr ) ? subResourceData.data() : nullptr, &nativeTextureObject->texture2D );

    if ( FAILED( operationResult ) ) {
        FLAN_CERR << "Texture creation failed! (error code " << operationResult << ")" << std::endl;
        return nullptr;
    }

    operationResult = nativeDevice->CreateShaderResourceView( nativeTextureObject->texture2D, &shaderResourceViewDesc, &nativeTextureObject->textureShaderResourceView );
    if ( FAILED( operationResult ) ) {
        FLAN_CERR << "Texture SRV creation failed! (error code " << operationResult << ")" << std::endl;
        return nullptr;
    }
   
    UpdateNativeTextureDescription( description, nativeTextureObject );

    return nativeTextureObject;
}

NativeTextureObject* flan::rendering::CreateTexture3DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
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
        D3D11_USAGE_DEFAULT,			// D3D11_USAGE Usage
        bindFlags,		                // UINT BindFlags
        static_cast<UINT>( 0 ),			// UINT CPUAccessFlags
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

    std::vector<D3D11_SUBRESOURCE_DATA> subResourceData( std::max( 1u, description.mipCount ) * description.arraySize );
    GetSubResourceDescriptor( subResourceData, initialData, description, nativeTextureFormat );

    NativeTextureObject* textureObject = new NativeTextureObject();
    auto nativeDevice = nativeRenderContext->nativeDevice;
    auto operationResult = nativeDevice->CreateTexture3D( &renderTargetDesc, initialData != nullptr ? subResourceData.data() : nullptr, &textureObject->texture3D );

    if ( FAILED( operationResult ) ) {
        FLAN_CERR << "Texture creation failed! (error code " << operationResult << ")" << std::endl;
        return nullptr;
    }

    nativeDevice->CreateShaderResourceView( textureObject->texture3D, &shaderResourceViewDesc, &textureObject->textureShaderResourceView );

    UpdateNativeTextureDescription( description, textureObject );
    return textureObject;
}

void flan::rendering::DestroyTextureImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject )
{
#define D3D11_RELEASE( obj ) if ( obj != nullptr ) { obj->Release(); obj = nullptr; }
    D3D11_RELEASE( textureObject->textureResource );
    D3D11_RELEASE( textureObject->textureShaderResourceView );
}

void flan::rendering::BindTextureCmdImpl( NativeCommandList* nativeCmdList, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
    auto nativeDeviceContext = nativeCmdList->deferredContext;
    if ( ( shaderStagesToBindTo & SHADER_STAGE_VERTEX ) == SHADER_STAGE_VERTEX ) {
        nativeDeviceContext->VSSetShaderResources( bindingIndex, 1, &textureObject->textureShaderResourceView );
    }

    if ( ( shaderStagesToBindTo & SHADER_STAGE_PIXEL ) == SHADER_STAGE_PIXEL ) {
        nativeDeviceContext->PSSetShaderResources( bindingIndex, 1, &textureObject->textureShaderResourceView );
    }

    if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_CONTROL ) == SHADER_STAGE_TESSELATION_CONTROL ) {
        nativeDeviceContext->DSSetShaderResources( bindingIndex, 1, &textureObject->textureShaderResourceView );
    }

    if ( ( shaderStagesToBindTo & SHADER_STAGE_TESSELATION_EVALUATION ) == SHADER_STAGE_TESSELATION_EVALUATION ) {
        nativeDeviceContext->HSSetShaderResources( bindingIndex, 1, &textureObject->textureShaderResourceView );
    }

    if ( ( shaderStagesToBindTo & SHADER_STAGE_COMPUTE ) == SHADER_STAGE_COMPUTE ) {
        nativeDeviceContext->CSSetShaderResources( bindingIndex, 1, &textureObject->textureShaderResourceView );
    }
}

void flan::rendering::UnbindTextureCmdImpl( NativeCommandList* nativeCmdList, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
    constexpr ID3D11ShaderResourceView* NO_SRV[1] = { ( ID3D11ShaderResourceView* )nullptr };

    auto nativeDeviceContext = nativeCmdList->deferredContext;
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

void flan::rendering::CopySubresouceRegionImpl( NativeRenderContext* nativeRenderContext, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject, const uint32_t mipSrc, const uint32_t arrayIdxSrc, const uint32_t mipDst, const uint32_t arrayIdxDst )
{
    auto nativeDeviceContext = nativeRenderContext->nativeDeviceContext;

    auto srcSubResource = D3D11CalcSubresource( mipSrc, arrayIdxSrc, srcTextureObject->textureMipCount );
    auto dstSubResource = D3D11CalcSubresource( mipDst, arrayIdxDst, dstTextureObject->textureMipCount );

    nativeDeviceContext->CopySubresourceRegion( dstTextureObject->textureResource, dstSubResource, 0, 0, 0, srcTextureObject->textureResource, srcSubResource, nullptr );
}

void flan::rendering::CopySubresouceRegionAsynchronousImpl( NativeCommandList* nativeCmdList, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject, const uint32_t mipSrc, const uint32_t arrayIdxSrc, const uint32_t mipDst, const uint32_t arrayIdxDst )
{
    auto nativeDeviceContext = nativeCmdList->deferredContext;

    auto srcSubResource = D3D11CalcSubresource( mipSrc, arrayIdxSrc, srcTextureObject->textureMipCount );;
    auto dstSubResource = D3D11CalcSubresource( mipDst, arrayIdxDst, dstTextureObject->textureMipCount );

    nativeDeviceContext->CopySubresourceRegion( dstTextureObject->textureResource, dstSubResource, 0, 0, 0, srcTextureObject->textureResource, srcSubResource, nullptr );
}

void flan::rendering::SetTextureDebugNameImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const std::string& debugName )
{
    textureObject->textureResource->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT)debugName.size(), debugName.c_str() );
}

void flan::rendering::RetrieveTextureTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<uint8_t>& texels )
{
    auto nativeDeviceContext = nativeRenderContext->nativeDeviceContext;

    D3D11_TEXTURE2D_DESC texDesc;
    textureObject->texture2D->GetDesc( &texDesc );

    ID3D11Texture2D* stagingTex = RetrieveStagingTexture( nativeRenderContext, textureObject, description );

    auto bpp = BitsPerPixel( texDesc.Format );
    std::size_t textureSize = 0;

    auto mipWidth = description.width, mipHeight = description.height;
    for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
        textureSize += ( mipWidth * bpp + 7 ) / 8 * mipHeight;

        mipWidth >>= 1;
        mipHeight >>= 1;
    }

    texels.resize( textureSize );

    D3D11_MAPPED_SUBRESOURCE msr;
    nativeDeviceContext->Map( stagingTex, 0, D3D11_MAP_READ, 0, &msr ); 
    memcpy( &texels[0], msr.pData, sizeof( uint8_t ) * textureSize );
    nativeDeviceContext->Unmap( stagingTex, 0 );

    stagingTex->Release();
}

void flan::rendering::RetrieveTextureLayerTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, const unsigned int layerIndex, const unsigned int mipLevel, std::vector<uint8_t>& texels )
{
    auto nativeDeviceContext = nativeRenderContext->nativeDeviceContext;

    D3D11_TEXTURE2D_DESC texDesc;
    textureObject->texture2D->GetDesc( &texDesc );

    ID3D11Texture2D* stagingTex = RetrieveStagingTexture( nativeRenderContext, textureObject, description );

    auto bpp = BitsPerPixel( texDesc.Format );
    std::size_t textureSize = ( ( description.width >> mipLevel ) * bpp + 7 ) / 8 * ( description.height >> mipLevel );

    std::size_t textureOffset = 0;
    unsigned int mipIndex = 0;
    auto mipWidth = description.width, mipHeight = description.height;
    while ( mipWidth > 0u && mipHeight > 0u && mipIndex < mipLevel ) {
        textureOffset += ( mipWidth * bpp + 7 ) / 8 * mipHeight;

        mipIndex++;
        mipWidth >>= 1;
        mipHeight >>= 1;
    }

    texels.resize( textureSize );

    D3D11_MAPPED_SUBRESOURCE msr;
    nativeDeviceContext->Map( stagingTex, 0, D3D11_MAP_READ, 0, &msr );
    memcpy( &texels[0], ( void* )( *( std::size_t* )msr.pData + textureOffset ), sizeof( uint8_t ) * textureSize );
    nativeDeviceContext->Unmap( stagingTex, 0 );

    stagingTex->Release();
}

void flan::rendering::RetrieveTextureTexelsHDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<float>& texels )
{
    auto nativeDeviceContext = nativeRenderContext->nativeDeviceContext;

    D3D11_TEXTURE2D_DESC texDesc;
    textureObject->texture2D->GetDesc( &texDesc );

    ID3D11Texture2D* stagingTex = RetrieveStagingTexture( nativeRenderContext, textureObject, description );

    auto bpp = BitsPerPixel( texDesc.Format );
    std::size_t textureSize = 0;

    auto mipWidth = description.width, mipHeight = description.height;
    while ( mipWidth > 0 && mipHeight > 0 ) {
        textureSize += ( mipWidth * bpp + 7 ) / 8 * mipHeight;

        mipWidth >>= 1;
        mipHeight >>= 1;
    }

    texels.resize( textureSize );

    D3D11_MAPPED_SUBRESOURCE msr;
    nativeDeviceContext->Map( stagingTex, 0, D3D11_MAP_READ, 0, &msr );
    memcpy( &texels[0], msr.pData, sizeof( float ) * textureSize );
    nativeDeviceContext->Unmap( stagingTex, 0 );

    stagingTex->Release();
}
#endif
