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

#if FLAN_GL460
#include <Rendering/RenderTarget.h>

#include <string>

struct NativeRenderContext;
struct NativeTextureObject;

struct NativeRenderTargetObject
{
};

namespace flan
{
    namespace rendering
    {
        NativeRenderTargetObject* CreateRenderTarget1DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, const NativeTextureObject* textureObject );
        NativeRenderTargetObject* CreateRenderTarget2DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, const NativeTextureObject* textureObject );
        NativeRenderTargetObject* CreateRenderTarget3DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, const NativeTextureObject* textureObject );

        void DestroyRenderTargetImpl( NativeRenderContext* nativeRenderContext, NativeRenderTargetObject* renderTargetObject );
        void SetRenderTargetDebugNameImpl( NativeRenderContext* nativeRenderContext, NativeRenderTargetObject* renderTargetObject, const std::string& debugName );
    }
}
#endif
