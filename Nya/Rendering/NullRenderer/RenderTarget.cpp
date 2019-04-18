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

RenderTarget* RenderDevice::createRenderTarget1D( const TextureDescription& description, Texture* initialTexture )
{
    return nullptr;
}

RenderTarget* RenderDevice::createRenderTarget2D( const TextureDescription& description, Texture* initialTexture )
{
    return nullptr;
}

RenderTarget* RenderDevice::createRenderTarget3D( const TextureDescription& description, Texture* initialTexture )
{
    return nullptr;
}

void RenderDevice::destroyRenderTarget( RenderTarget* renderTarget )
{

}

void CommandList::clearColorRenderTargets( RenderTarget** renderTargets, const uint32_t renderTargetCount, const float clearValue[4] )
{

}

void CommandList::clearDepthStencilRenderTarget( RenderTarget* renderTarget, const float clearValue )
{

}
#endif
