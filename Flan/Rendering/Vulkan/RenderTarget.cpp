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

#if FLAN_VULKAN
#include "RenderTarget.h"
#include "Texture.h"
#include "RenderContext.h"

#include <d3d11.h>

NativeRenderTargetObject* flan::rendering::CreateRenderTarget1DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, const NativeTextureObject* textureObject )
{
    NativeRenderTargetObject* nativeRenderTargetObject = new NativeRenderTargetObject();
    return nativeRenderTargetObject;
}

NativeRenderTargetObject* flan::rendering::CreateRenderTarget2DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, const NativeTextureObject* textureObject )
{
    NativeRenderTargetObject* nativeRenderTargetObject = new NativeRenderTargetObject();
    return nativeRenderTargetObject;
}

NativeRenderTargetObject* flan::rendering::CreateRenderTarget3DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, const NativeTextureObject* textureObject )
{
    NativeRenderTargetObject* nativeRenderTargetObject = new NativeRenderTargetObject();
    return nativeRenderTargetObject;
}

void flan::rendering::DestroyRenderTargetImpl( NativeRenderContext* nativeRenderContext, NativeRenderTargetObject* renderTargetObject )
{
}

void flan::rendering::SetRenderTargetDebugNameImpl( NativeRenderContext* nativeRenderContext, NativeRenderTargetObject* renderTargetObject, const std::string& debugName )
{
}
#endif
