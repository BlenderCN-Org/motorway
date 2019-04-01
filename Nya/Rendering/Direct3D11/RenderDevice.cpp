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
#include "RenderTarget.h"

#include <Display/DisplayMode.h>
#include <Display/DisplaySurface.h>

#if NYA_WIN
#include <Display/DisplaySurfaceWin32.h>
#endif

#include <Core/Allocators/PoolAllocator.h>

#include <d3d11.h>

using namespace nya::rendering;

RenderContext::RenderContext()
    : nativeDeviceContext( nullptr )
    , nativeDevice( nullptr )
    , swapChain( nullptr )
    , backbuffer( nullptr )
    , enableVsync( false )
    , renderPassAllocator( nullptr )
{

}

RenderContext::~RenderContext()
{
#define D3D11_RELEASE( object ) if ( object != nullptr ) { object->Release(); object = nullptr; }
    D3D11_RELEASE( backbuffer->textureRenderTargetView );
    D3D11_RELEASE( swapChain );
    D3D11_RELEASE( nativeDeviceContext );
    D3D11_RELEASE( nativeDevice );
#undef D3D11_RELEASE
}

RenderDevice::~RenderDevice()
{
    nya::core::freeArray<CommandList>( memoryAllocator, renderContext->cmdListPool );
    nya::core::freeArray<ResourceList>( memoryAllocator, renderContext->resListPool );

    nya::core::free( memoryAllocator, renderContext->renderPassAllocator );
    nya::core::free( memoryAllocator, renderContext );
}

void RenderDevice::create( DisplaySurface* surface )
{
    NYA_CLOG << "Creating RenderDevice (Direct3D 11)" << std::endl;

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

    NYA_ASSERT( ( bestAdapterIndex != UINT32_MAX ), "%s:%i >> D3D11: No adapters found!", NYA_FILENAME, __LINE__ );

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

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {
        {																					// DXGI_MODE_DESC BufferDesc
            nativeDisplaySurface->WindowWidth,												    //		UINT Width
            nativeDisplaySurface->WindowHeight,												    //		UINT Height
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
        nativeDisplaySurface->Handle,														// HWND OutputWindow
        static_cast<BOOL>( surface->displayMode == nya::display::eDisplayMode::WINDOWED ),	// BOOL Windowed
        DXGI_SWAP_EFFECT_DISCARD,															// DXGI_SWAP_EFFECT SwapEffect
        0,																					// UINT Flags
    };

    UINT creationFlags = 0;

#if NYA_DEVBUILD
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#else
    NYA_CLOG << "Debug Layer is disabled (build was compiled with NYA_NO_DEBUG_DEVICE)" << std::endl;
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

    NYA_CLOG << "D3D11CreateDeviceAndSwapChain >> Operation result: " << NYA_PRINT_HEX( nativeDeviceCreationResult ) << std::endl;

    D3D11_FEATURE_DATA_THREADING threadingInfos = { 0 };

    NYA_ASSERT( SUCCEEDED( nativeDevice->CheckFeatureSupport( D3D11_FEATURE_THREADING, &threadingInfos, sizeof( D3D11_FEATURE_DATA_THREADING ) ) ),
        "%s:%i >> D3D11: Failed to retrieve device features!", NYA_FILENAME, __LINE__ );
    NYA_ASSERT( threadingInfos.DriverCommandLists, "%s:%i >> D3D11: Device does not support multithreading!", NYA_FILENAME, __LINE__ );

    // It should be safe to release the adapter info after the device creation
    adapter->Release();

    renderContext = nya::core::allocate<RenderContext>( memoryAllocator );
    renderContext->nativeDevice = nativeDevice;
    renderContext->nativeDeviceContext = nativeDeviceContext;
    renderContext->swapChain = swapChain;
    renderContext->backbuffer = nya::core::allocate<RenderTarget>( memoryAllocator );

    // Unbind backbuffer
    nativeDeviceContext->OMSetRenderTargets( 0, 0, 0 );

#define RELEASE_D3D_RESOURCE( res ) if ( res != nullptr ) { res->Release(); res = nullptr; }
    // Release backbuffer resources
    RELEASE_D3D_RESOURCE( renderContext->backbuffer->textureRenderTargetView )

    swapChain->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0 );

    // Recreate the render target
    ID3D11Texture2D* backbufferTex2D = nullptr;
    swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&backbufferTex2D );
    nativeDevice->CreateRenderTargetView( backbufferTex2D, NULL, &renderContext->backbuffer->textureRenderTargetView );

    RELEASE_D3D_RESOURCE( backbufferTex2D )
#undef RELEASE_D3D_RESOURCE

    renderContext->cmdListPool = nya::core::allocateArray<CommandList>( memoryAllocator, 16, memoryAllocator );
    for ( size_t i = 0; i < 16; i++ ) {
        renderContext->cmdListPool[i].CommandListObject = nya::core::allocate<NativeCommandList>( memoryAllocator );
        renderContext->cmdListPool[i].CommandListObject->swapchain = renderContext->backbuffer;

        renderContext->nativeDevice->CreateDeferredContext( 0, &renderContext->cmdListPool[i].CommandListObject->deferredContext );
    }

    renderContext->cmdListPoolCapacity = 16;
    renderContext->cmdListPoolIndex = 0;

    renderContext->resListPool = nya::core::allocateArray<ResourceList>( memoryAllocator, 64 );
    renderContext->resListPoolCapacity = 64;
    renderContext->resListPoolIndex = 0;

    renderContext->renderPassAllocator = nya::core::allocate<PoolAllocator>( 
        memoryAllocator, 
        sizeof( RenderPass ),
        4, 
        sizeof( RenderPass ) * 48,
        memoryAllocator->allocate( sizeof( RenderPass ) * 48 )
    );
}

void RenderDevice::enableVerticalSynchronisation( const bool enabled )
{
    renderContext->enableVsync = enabled;
}

RenderTarget* RenderDevice::getSwapchainBuffer()
{
    return renderContext->backbuffer;
}

CommandList& RenderDevice::allocateGraphicsCommandList() const
{
    const auto cmdListIdx = renderContext->cmdListPoolIndex;
    renderContext->cmdListPoolIndex = ( ++renderContext->cmdListPoolIndex % renderContext->cmdListPoolCapacity );

    return renderContext->cmdListPool[cmdListIdx];
}

CommandList& RenderDevice::allocateComputeCommandList() const
{
    // D3D11 makes no difference between compute and graphics cmd list
    return allocateGraphicsCommandList();
}

void RenderDevice::submitCommandList( CommandList* commandList )
{
    NYA_DEV_ASSERT( commandList->CommandListObject->commandList != nullptr, "%s:%i >> D3D11: Command list was nullptr!", NYA_FILENAME, __LINE__ );

    renderContext->nativeDeviceContext->ExecuteCommandList( commandList->CommandListObject->commandList, FALSE );

    // Release CmdList (afaik you can't recycle them in d3d11)
    commandList->CommandListObject->commandList->Release();
}

void RenderDevice::present()
{
    NYA_PROFILE_FUNCTION

    HRESULT swapBufferResult = renderContext->swapChain->Present( ( renderContext->enableVsync ) ? 1 : 0, 0 );
    if ( FAILED( swapBufferResult ) ) {
        NYA_TRIGGER_BREAKPOINT
        NYA_CERR << "Failed to swap buffers! (error code " << NYA_PRINT_HEX( swapBufferResult ) << ")" << std::endl;
        NYA_ASSERT( swapBufferResult == DXGI_ERROR_DEVICE_REMOVED, "Device removed! (error code: 0%X)", renderContext->nativeDevice->GetDeviceRemovedReason() );
    }
}

const nyaChar_t* RenderDevice::getBackendName() const
{
    return NYA_STRING( "Direct3D 11" );
}
#endif
