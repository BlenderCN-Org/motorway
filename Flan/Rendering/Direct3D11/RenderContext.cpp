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
#include "RenderContext.h"

#include <Display/DisplaySurface.h>
#include <Display/DisplaySurfaceWin32.h>

#include <Rendering/RenderTarget.h>
#include <Rendering/PrimitiveTopologies.h>
#include <Rendering/ShaderStages.h>

#include "Texture.h"
#include "PipelineState.h"
#include "RenderTarget.h"

#include <d3d11.h>

void flan::rendering::PresentImpl( NativeRenderContext* nativeRenderContext )
{
    HRESULT swapBufferResult = S_OK;
    if ( FAILED( swapBufferResult = nativeRenderContext->swapChain->Present( ( nativeRenderContext->enableVsync ) ? 1 : 0, 0 ) ) ) {
        FLAN_CERR << "Failed to swap buffers! (error code 0x" << std::hex << swapBufferResult << std::dec << ")" << std::endl;

        if ( swapBufferResult == DXGI_ERROR_DEVICE_REMOVED ) {
            FLAN_CERR << "Reason: 0x" << std::hex << nativeRenderContext->nativeDevice->GetDeviceRemovedReason() << std::dec << std::endl;
        }
    }
}

void flan::rendering::SetVSyncStateImpl( NativeRenderContext* nativeRenderContext, const bool enabled )
{
    nativeRenderContext->enableVsync = enabled;
}

NativeRenderContext* flan::rendering::CreateRenderContextImpl( DisplaySurface* surface )
{
    FLAN_CLOG << "Creating RenderDevice (Direct3D 11)" << std::endl;

    IDXGIFactory1* factory = nullptr;
    CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), ( void** )&factory );

    FLAN_CLOG << "Enumerating Adapters..." << std::endl;

    UINT bestAdapterIndex = UINT32_MAX;
    SIZE_T bestVRAM = 0;

    IDXGIAdapter1* adapter = nullptr;
    IDXGIOutput* output = nullptr;
    DXGI_ADAPTER_DESC1 adapterDescription = {};

    // Enumerate adapters available on the system and choose the best one
    for ( UINT i = 0; factory->EnumAdapters1( i, &adapter ) != DXGI_ERROR_NOT_FOUND; ++i ) {
        adapter->GetDesc1( &adapterDescription );

        UINT outputCount = 0;
        for ( ; adapter->EnumOutputs( outputCount, &output ) != DXGI_ERROR_NOT_FOUND; ++outputCount );

        // VRAM (MB)
        const SIZE_T adapterVRAM = ( adapterDescription.DedicatedVideoMemory >> 20 );

        // Choose the best adapter based on available VRAM and adapter count
        if ( outputCount != 0 && adapterVRAM > bestVRAM ) {
            bestVRAM = adapterVRAM;
            bestAdapterIndex = i;
        }

        FLAN_CLOG << "-Adapter " << i << " "
            << adapterDescription.Description
            << " VRAM: " << adapterVRAM << "MB"
            << " (" << outputCount << " output(s) found)" << std::endl;
    }

    // TODO Throw an exception, display scary stuff on screen, ...
    if ( bestAdapterIndex == UINT32_MAX ) {
        FLAN_CERR << "No adapters found!" << std::endl;
        return nullptr;
    }

    factory->EnumAdapters1( bestAdapterIndex, &adapter );
    adapter->EnumOutputs( 0, &output );

    FLAN_CLOG << "Selected Adapter >> Adapter " << bestAdapterIndex << std::endl;

    DXGI_OUTPUT_DESC outputDescription = {};
    output->GetDesc( &outputDescription );
    FLAN_CLOG << "Selected Output >> " << outputDescription.DeviceName << std::endl;

    UINT vsyncNumerator = 0, vsyncDenominator = 0;
    UINT outputDisplayModeCount = 0;

    // Query display mode count
    output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &outputDisplayModeCount, nullptr );

    // Retrieve display mode list
    DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[outputDisplayModeCount]();
    output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &outputDisplayModeCount, displayModeList );

    auto nativeDisplaySurface = surface->getNativeDisplaySurface();

    for ( unsigned int i = 0; i < outputDisplayModeCount; ++i ) {
        if ( displayModeList[i].Width == nativeDisplaySurface->ClientWidth && displayModeList[i].Height == nativeDisplaySurface->ClientHeight ) {
            vsyncNumerator = displayModeList[i].RefreshRate.Numerator;
            vsyncDenominator = displayModeList[i].RefreshRate.Denominator;

            FLAN_CLOG << "Selected Display Mode >> " << displayModeList[i].Width << "x" << displayModeList[i].Height << " @ "
                << ( displayModeList[i].RefreshRate.Numerator / displayModeList[i].RefreshRate.Denominator ) << "Hz" << std::endl;
            break;
        }
    }

    delete[] displayModeList;

    output->Release();
    factory->Release();
   
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {
        {																					// DXGI_MODE_DESC BufferDesc
            nativeDisplaySurface->ClientWidth,												    //		UINT Width
            nativeDisplaySurface->ClientHeight,												    //		UINT Height
            {																				//		DXGI_RATIONAL RefreshRate
                vsyncNumerator,																//			UINT Numerator			
                vsyncDenominator,															//			UINT Denominator
            },																				//
            DXGI_FORMAT_R8G8B8A8_UNORM,										                //		DXGI_FORMAT Format
            DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,											//		DXGI_MODE_SCANLINE_ORDER ScanlineOrdering
            DXGI_MODE_SCALING_UNSPECIFIED,													//		DXGI_MODE_SCALING Scaling
        },																					//
        {																					// DXGI_SAMPLE_DESC SampleDesc
            1,																				//		UINT Count
            0,																				//		UINT Quality
        },																					//
        DXGI_USAGE_RENDER_TARGET_OUTPUT,													// DXGI_USAGE BufferUsage
        1,																					// UINT BufferCount
        nativeDisplaySurface->Handle,																// HWND OutputWindow
        static_cast<BOOL>( surface->getDisplayMode() == flan::core::eDisplayMode::WINDOWED ),					// BOOL Windowed
        DXGI_SWAP_EFFECT_DISCARD,															// DXGI_SWAP_EFFECT SwapEffect
        0,																					// UINT Flags
    };

    UINT creationFlags = 0;

#if FLAN_DEVBUILD
#ifndef FLAN_NO_DEBUG_DEVICE
    FLAN_IMPORT_VAR_PTR( EnableDebugRenderDevice, bool )
    if( *EnableDebugRenderDevice ) {
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
        FLAN_CWARN << "Debug Layer is enabled; performances might be impacted!" << std::endl;
    }
#else
    FLAN_CLOG << "Debug Layer is disabled (build was compiled with FLAN_NO_DEBUG_DEVICE)" << std::endl;
#endif
#endif

    ID3D11DeviceContext*	nativeDeviceContext = nullptr;
    ID3D11Device*			nativeDevice = nullptr;
    IDXGISwapChain*			swapChain = nullptr;

    static constexpr D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    const HRESULT nativeDeviceCreationResult = D3D11CreateDeviceAndSwapChain(
        adapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        NULL,
        creationFlags,
        &featureLevel,
        1,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &swapChain,
        &nativeDevice,
        NULL,
        &nativeDeviceContext
    );

    FLAN_CLOG << "D3D11CreateDeviceAndSwapChain >> Operation result: " << std::hex << "0x" << nativeDeviceCreationResult << std::dec << std::endl;

    D3D11_FEATURE_DATA_THREADING threadingInfos = { 0 };
    if ( FAILED( nativeDevice->CheckFeatureSupport( D3D11_FEATURE_THREADING, &threadingInfos, sizeof( D3D11_FEATURE_DATA_THREADING ) ) ) ) {
        FLAN_CERR << "Failed to retrieve device feature! Stopping here" << std::endl;
        return nullptr;
    }

    if ( !threadingInfos.DriverCommandLists ) {
        FLAN_CERR << "Device does not support multithreading! Stopping here" << std::endl;
        return nullptr;
    }

    // It should be safe to release the adapter info after the device creation
    adapter->Release();

    auto nativeRenderContext = new NativeRenderContext();
    nativeRenderContext->nativeDevice = nativeDevice;
    nativeRenderContext->nativeDeviceContext = nativeDeviceContext;
    nativeRenderContext->swapChain = swapChain;
    nativeRenderContext->backbuffer = nullptr;

    // ResizeFunc
    // Unbind backbuffer
    nativeDeviceContext->OMSetRenderTargets( 0, 0, 0 );

#define RELEASE_D3D_RESOURCE( res ) if ( res != nullptr ) { res->Release(); res = nullptr; }
    // Release backbuffer resources
    RELEASE_D3D_RESOURCE( nativeRenderContext->backbuffer )

    swapChain->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0 );

    // Recreate the render target
    ID3D11Texture2D* backbufferTex2D = nullptr;
    swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&backbufferTex2D );
    nativeDevice->CreateRenderTargetView( backbufferTex2D, NULL, &nativeRenderContext->backbuffer );

    RELEASE_D3D_RESOURCE( backbufferTex2D )
#undef RELEASE_D3D_RESOURCE

    return nativeRenderContext;
}
#endif
