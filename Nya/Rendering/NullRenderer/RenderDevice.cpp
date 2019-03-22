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

#if NYA_NULL_RENDERER
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

RenderDevice::~RenderDevice()
{
}

void RenderDevice::create( DisplaySurface* surface )
{
    NYA_CLOG << "Creating RenderDevice (Null Renderer)" << std::endl;
}

void RenderDevice::enableVerticalSynchronisation( const bool enabled )
{

}

RenderTarget* RenderDevice::getSwapchainBuffer()
{
    return nullptr;
}

CommandList& RenderDevice::allocateGraphicsCommandList() const
{

}

CommandList& RenderDevice::allocateComputeCommandList() const
{

}

void RenderDevice::submitCommandList( CommandList* commandList )
{

}

void RenderDevice::present()
{

}

const nyaChar_t* RenderDevice::getBackendName() const
{
    return NYA_STRING( "Null Renderer" );
}
#endif
