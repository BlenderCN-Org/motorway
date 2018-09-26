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

#include <d3d11.h>

#include <Rendering/Viewport.h>

class DisplaySurface;
class RenderTarget;
class BlendState;
class PipelineState;
class Texture;

struct NativeRenderContext
{
    ID3D11DeviceContext*	nativeDeviceContext;
    ID3D11Device*			nativeDevice;
    IDXGISwapChain*			swapChain;
    ID3D11RenderTargetView* backbuffer;
    float                   depthClearValue;

    ID3D11DepthStencilView* currentDepthBuffer;
    ID3D11RenderTargetView* currentRenderTargets[8];
    int                     currentRenderTargetCount;

    bool                    enableVsync;

    NativeRenderContext()
        : nativeDeviceContext( nullptr )
        , nativeDevice( nullptr )
        , swapChain( nullptr )
        , backbuffer( nullptr )
        , depthClearValue( 0.0f )
        , currentDepthBuffer( nullptr )
        , currentRenderTargets{ nullptr }
        , currentRenderTargetCount( 0 )
        , enableVsync( false )
    {

    }

    ~NativeRenderContext()
    {
#if FLAN_DEVBUILD
//#ifndef FLAN_NO_DEBUG_DEVICE
//        ID3D11Debug* debugInterface;
//        nativeDevice->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &debugInterface ) );
//        debugInterface->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
//        D3D11_RELEASE( debugInterface );
//#endif
#endif

#define D3D11_RELEASE( object ) if ( object != nullptr ) { object->Release(); object = nullptr; }
        D3D11_RELEASE( backbuffer );
        D3D11_RELEASE( swapChain );
        D3D11_RELEASE( nativeDeviceContext );
        D3D11_RELEASE( nativeDevice );
#undef D3D11_RELEASE

        currentDepthBuffer = nullptr;
        currentRenderTargetCount = 0;
    }
};

namespace flan
{
    namespace rendering
    {
        NativeRenderContext*    CreateRenderContextImpl( DisplaySurface* surface );
        void                    PresentImpl( NativeRenderContext* renderContext );
        void                    SetVSyncStateImpl( NativeRenderContext* nativeRenderContext, const bool enabled = false );
    }
}
#endif
