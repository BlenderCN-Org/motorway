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
#include "RenderTarget.h"
#include "Texture.h"
#include "RenderContext.h"

#include <d3d11.h>

ID3D11RenderTargetView* CreateRenderTargetView( ID3D11Device* device, ID3D11Resource* texResource, const D3D11_RTV_DIMENSION dimension, const DXGI_FORMAT format )
{
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {
        format,	    // DXGI_FORMAT Format
        dimension,  // D3D11_RTV_DIMENSION ViewDimension
        0,			// UINT Flags
    };

    ID3D11RenderTargetView* renderTargetView = nullptr;

    HRESULT creationResult = S_OK;
    if ( creationResult = FAILED( device->CreateRenderTargetView( texResource, &renderTargetViewDesc, &renderTargetView ) ) ) {
        FLAN_CERR << "RenderTarget Creation failed! (error code 0x" << std::hex << creationResult << std::dec << ")" << std::endl;
        return nullptr;
    }

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

    HRESULT creationResult = S_OK;
    if ( creationResult = FAILED( device->CreateRenderTargetView( texResource, &renderTargetViewDesc, &renderTargetView ) ) ) {
        FLAN_CERR << "RenderTarget Creation failed! (error code 0x" << std::hex << creationResult << std::dec << ")" << std::endl;
        return nullptr;
    }

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
    HRESULT creationResult = S_OK;
    if ( creationResult = FAILED( device->CreateDepthStencilView( texResource, &depthStencilViewDesc, &depthStencilView ) ) ) {
        FLAN_CERR << "DepthStencilView Creation failed! (error code 0x" << std::hex << creationResult << std::dec << ")" << std::endl;
        return nullptr;
    }

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
    HRESULT creationResult = S_OK;
    if ( FAILED( creationResult = device->CreateDepthStencilView( texResource, &depthStencilViewDesc, &depthStencilView ) ) ) {
        FLAN_CERR << "DepthStencilView Creation failed! (error code 0x" << std::hex << creationResult << std::dec << ")" << std::endl;
        return nullptr;
    }

    return depthStencilView;
}

NativeRenderTargetObject* flan::rendering::CreateRenderTarget1DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, const NativeTextureObject* textureObject )
{
    auto nativeDevice = nativeRenderContext->nativeDevice;

    NativeRenderTargetObject* nativeRenderTargetObject = new NativeRenderTargetObject();
    nativeRenderTargetObject->description = description;
    DXGI_FORMAT nativeTextureFormat = static_cast< DXGI_FORMAT >( description.format );

    if ( description.flags.isDepthResource == 1 ) {
        nativeRenderTargetObject->textureDepthRenderTargetView = CreateDepthRenderTargetView( nativeDevice, textureObject->texture1D, D3D11_DSV_DIMENSION_TEXTURE1D, nativeTextureFormat );

        nativeRenderTargetObject->textureDepthStencilViewPerSlice = new ID3D11DepthStencilView*[description.arraySize]{ nullptr };
        nativeRenderTargetObject->textureDepthStencilViewPerSliceAndMipLevel = new ID3D11DepthStencilView**[description.arraySize]{ nullptr };
        for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
            nativeRenderTargetObject->textureDepthStencilViewPerSlice[layerIdx] = CreateDepthRenderTargetViewArraySlice( nativeDevice, textureObject->texture1D, D3D11_DSV_DIMENSION_TEXTURE1DARRAY, nativeTextureFormat, 0, layerIdx );

            nativeRenderTargetObject->textureDepthStencilViewPerSliceAndMipLevel[layerIdx] = new ID3D11DepthStencilView*[description.mipCount]{ nullptr };
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                nativeRenderTargetObject->textureDepthStencilViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateDepthRenderTargetViewArraySlice( nativeDevice, textureObject->texture1D, D3D11_DSV_DIMENSION_TEXTURE1DARRAY, nativeTextureFormat, mipLevel, layerIdx );
            }
        }
    } else {
        nativeRenderTargetObject->textureRenderTargetView = CreateRenderTargetView( nativeDevice, textureObject->texture1D, D3D11_RTV_DIMENSION_TEXTURE1D, nativeTextureFormat );

        nativeRenderTargetObject->textureRenderTargetViewPerSlice = new ID3D11RenderTargetView*[description.arraySize]{ nullptr };
        nativeRenderTargetObject->textureRenderTargetViewPerSliceAndMipLevel = new ID3D11RenderTargetView**[description.arraySize]{ nullptr };
        for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
            nativeRenderTargetObject->textureRenderTargetViewPerSlice[layerIdx] = CreateRenderTargetViewArraySlice( nativeDevice, textureObject->texture1D, D3D11_RTV_DIMENSION_TEXTURE1DARRAY, nativeTextureFormat, 0, layerIdx );

            nativeRenderTargetObject->textureRenderTargetViewPerSliceAndMipLevel[layerIdx] = new ID3D11RenderTargetView*[description.mipCount]{ nullptr };
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                nativeRenderTargetObject->textureRenderTargetViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateRenderTargetViewArraySlice( nativeDevice, textureObject->texture1D, D3D11_RTV_DIMENSION_TEXTURE1DARRAY, nativeTextureFormat, mipLevel, layerIdx );
            }
        }
    }

    return nativeRenderTargetObject;
}

NativeRenderTargetObject* flan::rendering::CreateRenderTarget2DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, const NativeTextureObject* textureObject )
{
    auto nativeDevice = nativeRenderContext->nativeDevice;

    NativeRenderTargetObject* nativeRenderTargetObject = new NativeRenderTargetObject();
    nativeRenderTargetObject->description = description;

    DXGI_FORMAT nativeTextureFormat = static_cast< DXGI_FORMAT >( description.format );

    if ( description.flags.isDepthResource == 1 ) {
        const D3D11_DSV_DIMENSION dimension = ( description.samplerCount <= 1 ) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
        const D3D11_DSV_DIMENSION arrayDimension = ( description.samplerCount <= 1 ) ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
        nativeRenderTargetObject->textureDepthRenderTargetView = CreateDepthRenderTargetView( nativeDevice, textureObject->texture2D, dimension, nativeTextureFormat );

        nativeRenderTargetObject->textureDepthStencilViewPerSlice = new ID3D11DepthStencilView*[description.arraySize]{ nullptr };
        nativeRenderTargetObject->textureDepthStencilViewPerSliceAndMipLevel = new ID3D11DepthStencilView**[description.arraySize]{ nullptr };
        for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
            nativeRenderTargetObject->textureDepthStencilViewPerSlice[layerIdx] = CreateDepthRenderTargetViewArraySlice( nativeDevice, textureObject->texture2D, arrayDimension, nativeTextureFormat, 0, layerIdx );

            nativeRenderTargetObject->textureDepthStencilViewPerSliceAndMipLevel[layerIdx] = new ID3D11DepthStencilView*[description.mipCount]{ nullptr };
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                nativeRenderTargetObject->textureDepthStencilViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateDepthRenderTargetViewArraySlice( nativeDevice, textureObject->texture2D, arrayDimension, nativeTextureFormat, mipLevel, layerIdx );
            }
        }
    } else {
        const D3D11_RTV_DIMENSION dimension = ( description.samplerCount <= 1 ) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
        const D3D11_RTV_DIMENSION arrayDimension = ( description.samplerCount <= 1 ) ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY : D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
        nativeRenderTargetObject->textureRenderTargetView = CreateRenderTargetView( nativeDevice, textureObject->texture2D, dimension, nativeTextureFormat );

        nativeRenderTargetObject->textureRenderTargetViewPerSlice = new ID3D11RenderTargetView*[description.arraySize]{ nullptr };
        nativeRenderTargetObject->textureRenderTargetViewPerSliceAndMipLevel = new ID3D11RenderTargetView**[description.arraySize]{ nullptr };
        for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
            nativeRenderTargetObject->textureRenderTargetViewPerSlice[layerIdx] = CreateRenderTargetViewArraySlice( nativeDevice, textureObject->texture2D, arrayDimension, nativeTextureFormat, 0, layerIdx );

            nativeRenderTargetObject->textureRenderTargetViewPerSliceAndMipLevel[layerIdx] = new ID3D11RenderTargetView*[description.mipCount]{ nullptr };
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                nativeRenderTargetObject->textureRenderTargetViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateRenderTargetViewArraySlice( nativeDevice, textureObject->texture2D, arrayDimension, nativeTextureFormat, mipLevel, layerIdx );
            }
        }
    }

    return nativeRenderTargetObject;
}

NativeRenderTargetObject* flan::rendering::CreateRenderTarget3DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, const NativeTextureObject* textureObject )
{
    auto nativeDevice = nativeRenderContext->nativeDevice;

    NativeRenderTargetObject* nativeRenderTargetObject = new NativeRenderTargetObject();
    nativeRenderTargetObject->description = description;
    DXGI_FORMAT nativeTextureFormat = static_cast< DXGI_FORMAT >( description.format );

    if ( description.flags.isDepthResource == 1 ) {
        FLAN_CERR << "Invalid API use (3D depth rendertarget aren't supported)!" << std::endl;
    } else {
        nativeRenderTargetObject->textureRenderTargetView = CreateRenderTargetView( nativeDevice, textureObject->texture3D, D3D11_RTV_DIMENSION_TEXTURE3D, nativeTextureFormat );

        nativeRenderTargetObject->textureRenderTargetViewPerSlice = new ID3D11RenderTargetView*[description.arraySize]{ nullptr };
        nativeRenderTargetObject->textureRenderTargetViewPerSliceAndMipLevel = new ID3D11RenderTargetView**[description.arraySize]{ nullptr };
        for ( unsigned int layerIdx = 0; layerIdx < description.arraySize; layerIdx++ ) {
            nativeRenderTargetObject->textureRenderTargetViewPerSlice[layerIdx] = CreateRenderTargetViewArraySlice( nativeDevice, textureObject->texture3D, D3D11_RTV_DIMENSION_TEXTURE2DARRAY, nativeTextureFormat, 0, layerIdx );

            nativeRenderTargetObject->textureRenderTargetViewPerSliceAndMipLevel[layerIdx] = new ID3D11RenderTargetView*[description.mipCount]{ nullptr };
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                nativeRenderTargetObject->textureRenderTargetViewPerSliceAndMipLevel[layerIdx][mipLevel] = CreateRenderTargetViewArraySlice( nativeDevice, textureObject->texture2D, D3D11_RTV_DIMENSION_TEXTURE2DARRAY, nativeTextureFormat, mipLevel, layerIdx );
            }
        }
    }

    return nativeRenderTargetObject;
}

void flan::rendering::DestroyRenderTargetImpl( NativeRenderContext* nativeRenderContext, NativeRenderTargetObject* renderTargetObject )
{
#define D3D11_RELEASE( obj ) if ( obj != nullptr ) { obj->Release(); }

    if ( renderTargetObject->description.flags.isDepthResource == 1 ) {
        D3D11_RELEASE( renderTargetObject->textureDepthRenderTargetView );

        for ( unsigned int layerIdx = 0; layerIdx < renderTargetObject->description.arraySize; layerIdx++ ) {
            D3D11_RELEASE( renderTargetObject->textureDepthStencilViewPerSlice[layerIdx] );

            for ( unsigned int mipLevel = 0; mipLevel < renderTargetObject->description.mipCount; mipLevel++ ) {
                D3D11_RELEASE( renderTargetObject->textureDepthStencilViewPerSliceAndMipLevel[layerIdx][mipLevel] );
            }

            delete[] renderTargetObject->textureDepthStencilViewPerSliceAndMipLevel[layerIdx];
        }

        delete[] renderTargetObject->textureDepthStencilViewPerSlice;
        delete[] renderTargetObject->textureDepthStencilViewPerSliceAndMipLevel;
    } else {
        D3D11_RELEASE( renderTargetObject->textureRenderTargetView );

        for ( unsigned int layerIdx = 0; layerIdx < renderTargetObject->description.arraySize; layerIdx++ ) {
            D3D11_RELEASE( renderTargetObject->textureRenderTargetViewPerSlice[layerIdx] );

            for ( unsigned int mipLevel = 0; mipLevel < renderTargetObject->description.mipCount; mipLevel++ ) {
                D3D11_RELEASE( renderTargetObject->textureRenderTargetViewPerSliceAndMipLevel[layerIdx][mipLevel] );
            }

            delete[] renderTargetObject->textureRenderTargetViewPerSliceAndMipLevel[layerIdx];
        }

        delete[] renderTargetObject->textureRenderTargetViewPerSlice;
        delete[] renderTargetObject->textureRenderTargetViewPerSliceAndMipLevel;
    }
}

void flan::rendering::SetRenderTargetDebugNameImpl( NativeRenderContext* nativeRenderContext, NativeRenderTargetObject* renderTargetObject, const std::string& debugName )
{
    renderTargetObject->textureRenderTargetView->SetPrivateData( WKPDID_D3DDebugObjectName, ( UINT )debugName.size(), debugName.c_str() );
}
#endif
