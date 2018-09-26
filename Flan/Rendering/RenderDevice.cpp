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
#include "RenderDevice.h"

#include <Display/DisplaySurface.h>

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#endif

#include "PipelineState.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"

FLAN_ENV_VAR( EnableVSync, "Enables Vertical Synchronisation [false/true]", true, bool )

RenderDevice::RenderDevice()
    : nativeRenderContext( nullptr )
    , currentRasterizerState( 0 )
    , currentBlendState( 0 )
{

}

RenderDevice::~RenderDevice()
{

}

NativeRenderContext* RenderDevice::getNativeRenderContext() const
{
    return nativeRenderContext.get();
}

void RenderDevice::create( DisplaySurface* surface )
{
    nativeRenderContext.reset( flan::rendering::CreateRenderContextImpl( surface ) );
}

void RenderDevice::present()
{
    flan::rendering::PresentImpl( nativeRenderContext.get() );
}

void RenderDevice::setVSyncState( const bool enabled )
{
    EnableVSync = enabled;

    flan::rendering::SetVSyncStateImpl( nativeRenderContext.get(), EnableVSync );
}
