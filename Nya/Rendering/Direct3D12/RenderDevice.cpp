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

#if NYA_D3D12
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include "RenderDevice.h"

#include <Display/DisplayMode.h>
#include <Display/DisplaySurface.h>

#if NYA_WIN
#include <Display/DisplaySurfaceWin32.h>
#endif

#include <d3d12.h>
#include <dxgi.h>

using namespace nya::rendering;

RenderContext::RenderContext()
    : nativeDevice( nullptr )
{

}

RenderContext::~RenderContext()
{

}

RenderDevice::~RenderDevice()
{

}

void RenderDevice::create( DisplaySurface* surface )
{
    NYA_CLOG << "Creating RenderDevice (Direct3D 12)" << std::endl;

    IDXGIFactory1* factory = nullptr;
    CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), ( void** )&factory );

    NYA_CLOG << "Enumerating Adapters..." << std::endl;

    UINT bestAdapterIndex = UINT32_MAX;
    UINT bestOutputCount = 0;
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
            bestOutputCount = outputCount;
        }

        NYA_CLOG << "-Adapter " << i << " '"
            << adapterDescription.Description
            << "' VRAM: " << adapterVRAM << "MB"
            << " (" << outputCount << " output(s) found)" << std::endl;
    }

    NYA_ASSERT( ( bestAdapterIndex != UINT32_MAX ), "%s:%i >> D3D12: No adapters found!", NYA_FILENAME, __LINE__ );

    factory->EnumAdapters1( bestAdapterIndex, &adapter );

    NYA_CLOG << "Selected Adapter >> Adapter " << bestAdapterIndex << std::endl;

    NYA_CLOG << "Enumerating Outputs..." << std::endl;
    for ( UINT outputIdx = 0; outputIdx < bestOutputCount; outputIdx++ ) {
        adapter->EnumOutputs( outputIdx, &output );

        DXGI_OUTPUT_DESC outputDescription = {};
        output->GetDesc( &outputDescription );
        
        NYA_CLOG << "-Output " << outputIdx << " '"
            << outputDescription.DeviceName
            << "' at " << outputDescription.DesktopCoordinates.left << "x"
            << outputDescription.DesktopCoordinates.top << std::endl;
    }

    UINT vsyncNumerator = 0, vsyncDenominator = 0;
    UINT vsyncRefreshRate = 0;
    UINT outputDisplayModeCount = 0;

    // Query display mode count
    output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &outputDisplayModeCount, nullptr );

    // Retrieve display mode list
    DXGI_MODE_DESC displayModeList[128];
    output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &outputDisplayModeCount, displayModeList );

    auto nativeDisplaySurface = surface->nativeDisplaySurface;

    for ( unsigned int i = 0; i < outputDisplayModeCount; ++i ) {
        UINT refreshRate = ( displayModeList[i].RefreshRate.Numerator / displayModeList[i].RefreshRate.Denominator );

        if ( displayModeList[i].Width == nativeDisplaySurface->WindowWidth
            && displayModeList[i].Height == nativeDisplaySurface->WindowHeight
            && refreshRate > vsyncRefreshRate ) {
            vsyncNumerator = displayModeList[i].RefreshRate.Numerator;
            vsyncDenominator = displayModeList[i].RefreshRate.Denominator;
            vsyncRefreshRate = refreshRate;
        }
    }

    NYA_CLOG << "Selected Display Mode >> " << nativeDisplaySurface->WindowWidth << "x" << nativeDisplaySurface->WindowHeight << " @ "
        << vsyncRefreshRate << "Hz" << std::endl;

    output->Release();
    factory->Release();

    ID3D12Device* nativeDevice = nullptr;
    const HRESULT nativeDeviceCreationResult = D3D12CreateDevice(
        adapter,
        D3D_FEATURE_LEVEL_12_0,
        _uuidof( ID3D12Device ), 
        reinterpret_cast<void**>( &nativeDevice )
    );

    NYA_CLOG << "D3D12CreateDevice >> Operation result: " << NYA_PRINT_HEX( nativeDeviceCreationResult ) << std::endl;
}

void RenderDevice::enableVerticalSynchronisation( const bool enabled )
{
    renderContext->enableVsync = enabled;
}

RenderTarget* RenderDevice::getSwapchainBuffer()
{
    return nullptr;
}

CommandList& RenderDevice::allocateGraphicsCommandList() const
{
    return {};
}

CommandList& RenderDevice::allocateComputeCommandList() const
{
    return {};
}

void RenderDevice::submitCommandList( CommandList* commandList )
{

}

void RenderDevice::present()
{
    HRESULT swapBufferResult = renderContext->swapChain->Present( ( renderContext->enableVsync ) ? 1 : 0, 0 );
    if ( FAILED( swapBufferResult ) ) {
        NYA_TRIGGER_BREAKPOINT
        NYA_CERR << "Failed to swap buffers! (error code " << NYA_PRINT_HEX( swapBufferResult ) << ")" << std::endl;
        NYA_ASSERT( swapBufferResult == DXGI_ERROR_DEVICE_REMOVED, "Device removed! (error code: 0%X)", renderContext->nativeDevice->GetDeviceRemovedReason() );
    }
}

const nyaChar_t* RenderDevice::getBackendName() const
{
    return NYA_STRING( "Direct3D 12" );
}
#endif
