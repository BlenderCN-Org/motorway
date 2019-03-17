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

#include "RenderDevice.h"
#include "CommandList.h"

#include "Texture.h"
#include "RenderTarget.h"

#include <d3d11.h>

ID3D11RenderTargetView* CreateRenderTargetView( ID3D11Device* device, ID3D11Resource* texResource, const D3D11_RTV_DIMENSION dimension, const DXGI_FORMAT format )
{
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {
        format,	    // DXGI_FORMAT Format
        dimension,  // D3D11_RTV_DIMENSION ViewDimension
        0,			// UINT Flags
    };

    ID3D11RenderTargetView* renderTargetView = nullptr;

    HRESULT creationResult = device->CreateRenderTargetView( texResource, &renderTargetViewDesc, &renderTargetView );
   
    return renderTargetView;
}

ID3D11RenderTargetView* CreateRenderTargetViewArraySlice( ID3D11Device* device, ID3D11Resource* texResource, const D3D11_RTV_DIMENSION dimension, const DXGI_FORMAT format, const UINT mipSlice, const UINT firstArraySlice )
{
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {
        format,	    // DXGI_FORMAT Format
        dimension,  // D3D11_RTV_DIMENSION ViewDimension
        0,			// UINT Flags
    };

    // NOTE Dont care about view format (API use union for every format)
    if ( dimension == D3D11_RTV_DIMENSION_TEXTURE2DARRAY ) {
        renderTargetViewDesc.Texture2DArray.MipSlice = mipSlice;
        renderTargetViewDesc.Texture2DArray.FirstArraySlice = firstArraySlice;
        renderTargetViewDesc.Texture2DArray.ArraySize = 1;
    } else if ( dimension == D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY ) {
        //
        renderTargetViewDesc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
        renderTargetViewDesc.Texture2DMSArray.ArraySize = 1;
    }

    ID3D11RenderTargetView* renderTargetView = nullptr;

    HRESULT creationResult = device->CreateRenderTargetView( texResource, &renderTargetViewDesc, &renderTargetView );

    return renderTargetView;
}

ID3D11DepthStencilView* CreateDepthRenderTargetView( ID3D11Device* device, ID3D11Resource* texResource, const D3D11_DSV_DIMENSION dimension, const DXGI_FORMAT format )
{
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {
        format,	// DXGI_FORMAT Format
        dimension,			// D3D11_DSV_DIMENSION ViewDimension
        0,						// UINT Flags
    };

    switch ( format ) {
    case DXGI_FORMAT_R16_TYPELESS:
        depthStencilViewDesc.Format = DXGI_FORMAT_D16_UNORM;
        break;
    case DXGI_FORMAT_R32_TYPELESS:
        depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
        break;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        break;
    }

    ID3D11DepthStencilView* depthStencilView = nullptr;
    HRESULT creationResult = device->CreateDepthStencilView( texResource, &depthStencilViewDesc, &depthStencilView );
   
    return depthStencilView;
}

ID3D11DepthStencilView* CreateDepthRenderTargetViewArraySlice( ID3D11Device* device, ID3D11Resource* texResource, const D3D11_DSV_DIMENSION dimension, const DXGI_FORMAT format, const UINT mipSlice, const UINT firstArraySlice )
{
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {
        format,	    // DXGI_FORMAT Format
        dimension,  // D3D11_DSV_DIMENSION ViewDimension
        0,			// UINT Flags
    };

    switch ( format ) {
    case DXGI_FORMAT_R16_TYPELESS:
        depthStencilViewDesc.Format = DXGI_FORMAT_D16_UNORM;
        break;
    case DXGI_FORMAT_R32_TYPELESS:
        depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
        break;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        break;
    }

    // NOTE Dont care about view format (API use union for every format)
    if ( dimension == D3D11_DSV_DIMENSION_TEXTURE2DARRAY ) {
        depthStencilViewDesc.Texture2DArray.MipSlice = mipSlice;
        depthStencilViewDesc.Texture2DArray.FirstArraySlice = firstArraySlice;
        depthStencilViewDesc.Texture2DArray.ArraySize = 1;
    } else if ( dimension == D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY ) {
        depthStencilViewDesc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
        depthStencilViewDesc.Texture2DMSArray.ArraySize = 1;
    }

    ID3D11DepthStencilView* depthStencilView = nullptr;
    HRESULT creationResult = device->CreateDepthStencilView( texResource, &depthStencilViewDesc, &depthStencilView );

    return depthStencilView;
}

RenderTarget* RenderDevice::createRenderTarget1D( const TextureDescription& description, Texture* initialTexture )
{
    RenderTarget* renderTarget = nya::core::allocate<RenderTarget>( memoryAllocator );
    renderTarget->texture = ( initialTexture != nullptr ) ? initialTexture : createTexture1D( description );

    ID3D11Device* nativeDevice = renderContext->nativeDevice;

    DXGI_FORMAT nativeTextureFormat = static_cast< DXGI_FORMAT >( description.format );
    if ( description.flags.isDepthResource == 1 ) {
        renderTarget->textureDepthRenderTargetView = CreateDepthRenderTargetView( nativeDevice, renderTarget->texture->texture1D, D3D11_DSV_DIMENSION_TEXTURE1D, nativeTextureFormat );

        renderTarget->textureDepthStencilViewPerSlice = nya::core::allocateArray<ID3D11DepthStencilView*>( memoryAllocator, description.arraySize );
        renderTarget->textureDepthStencilViewPerSliceAndMipLevel = nya::core::allocateArray<ID3D11DepthStencilView**>( memoryAllocator, description.arraySize );
        for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
            renderTarget->textureDepthStencilViewPerSlice[layerIdx] = CreateDepthRenderTargetViewArraySlice( nativeDevice, renderTarget->texture->texture1D, D3D11_DSV_DIMENSION_TEXTURE1DARRAY, nativeTextureFormat, 0, layerIdx );

            renderTarget->textureDepthStencilViewPerSliceAndMipLevel[layerIdx] = nya::core::allocateArray<ID3D11DepthStencilView*>( memoryAllocator, description.mipCount );
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                renderTarget->textureDepthStencilViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateDepthRenderTargetViewArraySlice( nativeDevice, renderTarget->texture->texture1D, D3D11_DSV_DIMENSION_TEXTURE1DARRAY, nativeTextureFormat, mipLevel, layerIdx );
            }
        }
    } else {
        renderTarget->textureRenderTargetView = CreateRenderTargetView( nativeDevice, renderTarget->texture->texture1D, D3D11_RTV_DIMENSION_TEXTURE1D, nativeTextureFormat );

        renderTarget->textureRenderTargetViewPerSlice = nya::core::allocateArray<ID3D11RenderTargetView*>( memoryAllocator, description.arraySize );
        renderTarget->textureRenderTargetViewPerSliceAndMipLevel = nya::core::allocateArray<ID3D11RenderTargetView**>( memoryAllocator, description.arraySize );
        for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
            renderTarget->textureRenderTargetViewPerSlice[layerIdx] = CreateRenderTargetViewArraySlice( nativeDevice, renderTarget->texture->texture1D, D3D11_RTV_DIMENSION_TEXTURE1DARRAY, nativeTextureFormat, 0, layerIdx );

            renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx] = nya::core::allocateArray<ID3D11RenderTargetView*>( memoryAllocator, description.mipCount );
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateRenderTargetViewArraySlice( nativeDevice, renderTarget->texture->texture1D, D3D11_RTV_DIMENSION_TEXTURE1DARRAY, nativeTextureFormat, mipLevel, layerIdx );
            }
        }
    }

    return renderTarget;
}

RenderTarget* RenderDevice::createRenderTarget2D( const TextureDescription& description, Texture* initialTexture )
{
    RenderTarget* renderTarget = nya::core::allocate<RenderTarget>( memoryAllocator );
    renderTarget->texture = ( initialTexture != nullptr ) ? initialTexture : createTexture2D( description );

    ID3D11Device* nativeDevice = renderContext->nativeDevice;

    DXGI_FORMAT nativeTextureFormat = static_cast< DXGI_FORMAT >( description.format );
    if ( description.flags.isDepthResource == 1 ) {
        const D3D11_DSV_DIMENSION dimension = ( description.samplerCount <= 1 ) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
        const D3D11_DSV_DIMENSION arrayDimension = ( description.samplerCount <= 1 ) ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
        renderTarget->textureDepthRenderTargetView = CreateDepthRenderTargetView( nativeDevice, renderTarget->texture->texture2D, dimension, nativeTextureFormat );

        renderTarget->textureDepthStencilViewPerSlice = nya::core::allocateArray<ID3D11DepthStencilView*>( memoryAllocator, description.arraySize );
        renderTarget->textureDepthStencilViewPerSliceAndMipLevel = nya::core::allocateArray<ID3D11DepthStencilView**>( memoryAllocator, description.arraySize );
        for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
            renderTarget->textureDepthStencilViewPerSlice[layerIdx] = CreateDepthRenderTargetViewArraySlice( nativeDevice, renderTarget->texture->texture2D, arrayDimension, nativeTextureFormat, 0, layerIdx );

            renderTarget->textureDepthStencilViewPerSliceAndMipLevel[layerIdx] = nya::core::allocateArray<ID3D11DepthStencilView*>( memoryAllocator, description.mipCount );
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                renderTarget->textureDepthStencilViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateDepthRenderTargetViewArraySlice( nativeDevice, renderTarget->texture->texture2D, arrayDimension, nativeTextureFormat, mipLevel, layerIdx );
            }
        }
    } else {
        const D3D11_RTV_DIMENSION dimension = ( description.samplerCount <= 1 ) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
        const D3D11_RTV_DIMENSION arrayDimension = ( description.samplerCount <= 1 ) ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY : D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
        renderTarget->textureRenderTargetView = CreateRenderTargetView( nativeDevice, renderTarget->texture->texture2D, dimension, nativeTextureFormat );

        renderTarget->textureRenderTargetViewPerSlice = nya::core::allocateArray<ID3D11RenderTargetView*>( memoryAllocator, description.arraySize );
        renderTarget->textureRenderTargetViewPerSliceAndMipLevel = nya::core::allocateArray<ID3D11RenderTargetView**>( memoryAllocator, description.arraySize );
        for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
            renderTarget->textureRenderTargetViewPerSlice[layerIdx] = CreateRenderTargetViewArraySlice( nativeDevice, renderTarget->texture->texture2D, arrayDimension, nativeTextureFormat, 0, layerIdx );

            renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx] = nya::core::allocateArray<ID3D11RenderTargetView*>( memoryAllocator, description.mipCount );
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateRenderTargetViewArraySlice( nativeDevice, renderTarget->texture->texture2D, arrayDimension, nativeTextureFormat, mipLevel, layerIdx );
            }
        }
    }

    return renderTarget;
}

RenderTarget* RenderDevice::createRenderTarget3D( const TextureDescription& description, Texture* initialTexture )
{
    RenderTarget* renderTarget = nya::core::allocate<RenderTarget>( memoryAllocator );
    renderTarget->texture = ( initialTexture != nullptr ) ? initialTexture : createTexture3D( description );

    ID3D11Device* nativeDevice = renderContext->nativeDevice;

    DXGI_FORMAT nativeTextureFormat = static_cast< DXGI_FORMAT >( description.format );
    if ( description.flags.isDepthResource != 1 ) {
        renderTarget->textureRenderTargetView = CreateRenderTargetView( nativeDevice, renderTarget->texture->texture3D, D3D11_RTV_DIMENSION_TEXTURE3D, nativeTextureFormat );

        renderTarget->textureRenderTargetViewPerSlice = nya::core::allocateArray<ID3D11RenderTargetView*>( memoryAllocator, description.arraySize );
        renderTarget->textureRenderTargetViewPerSliceAndMipLevel = nya::core::allocateArray<ID3D11RenderTargetView**>( memoryAllocator, description.arraySize );
        for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
            renderTarget->textureRenderTargetViewPerSlice[layerIdx] = CreateRenderTargetViewArraySlice( nativeDevice, renderTarget->texture->texture3D, D3D11_RTV_DIMENSION_TEXTURE2DARRAY, nativeTextureFormat, 0, layerIdx );

            renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx] = nya::core::allocateArray<ID3D11RenderTargetView*>( memoryAllocator, description.mipCount );
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                renderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateRenderTargetViewArraySlice( nativeDevice, renderTarget->texture->texture2D, D3D11_RTV_DIMENSION_TEXTURE2DARRAY, nativeTextureFormat, mipLevel, layerIdx );
            }
        }
    }

    return renderTarget;
}

void RenderDevice::destroyRenderTarget( RenderTarget* renderTarget )
{
    renderTarget->textureRenderTargetView->Release();

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    renderTarget->texture->shaderResourceView->GetDesc( &srvDesc );

    // Retrieve mip/array size for rtv deletion
    int mipCount = 1, arraySize = 1;
    switch ( srvDesc.ViewDimension ) {
    case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE1DARRAY: {
        D3D11_TEXTURE1D_DESC texDesc;
        renderTarget->texture->texture1D->GetDesc( &texDesc );

        arraySize = texDesc.ArraySize;
        mipCount = texDesc.MipLevels;
        break;
    }

    case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:
    case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2DARRAY: {
        D3D11_TEXTURE2D_DESC texDesc;
        renderTarget->texture->texture2D->GetDesc( &texDesc );

        arraySize = texDesc.ArraySize;
        mipCount = texDesc.MipLevels;
        break;
    }

    case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE1D:
    case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D:
    case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE3D:
    case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURECUBE:
        mipCount = srvDesc.Texture1D.MipLevels;
        break;

    case D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:
        arraySize = srvDesc.Texture2DMSArray.ArraySize;
        break;
    }

    // Iterate and destroy rtv
    for ( int arrayIdx = 0; arrayIdx < arraySize; arrayIdx++ ) {
        renderTarget->textureRenderTargetViewPerSlice[arrayIdx]->Release();

        for ( int mipIdx = 0; mipIdx < mipCount; mipIdx++ ) {
            renderTarget->textureRenderTargetViewPerSliceAndMipLevel[arrayIdx][mipIdx]->Release();
        }
    }

    // Finally destroy bound texture
    destroyTexture( renderTarget->texture );

    nya::core::free( memoryAllocator, renderTarget );
}
#endif
