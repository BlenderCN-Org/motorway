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
#include <Rendering/DepthStencilState.h>

#include "Extensions.h"
struct NativeRenderContext;
struct NativeCommandList;

struct NativeDepthStencilStateObject
{
    NativeDepthStencilStateObject()
    {

    }

    bool        enableDepthTest;
    GLenum      enableDepthWrite;
    bool        enableStencilTest;
    bool        enableDepthBoundsTest;
    float       depthBoundsMin;
    float       depthBoundsMax;
    uint8_t     stencilRefValue;
    uint8_t     stencilWriteMask;
    uint8_t     stencilReadMask;
    GLenum      depthComparisonFunc;

    GLenum      stencilComparisonFunc;

    struct {
        GLenum    comparisonFunc;
        GLenum    passOp;
        GLenum    failOp;
        GLenum    zFailOp;
    } front, back;
};

namespace flan
{
    namespace rendering
    {
        NativeDepthStencilStateObject* CreateDepthStencilStateImpl( NativeRenderContext* nativeRenderContext, const DepthStencilStateDesc& description );
        void DestroyDepthStencilStateImpl( NativeRenderContext* nativeRenderContext, NativeDepthStencilStateObject* depthStencilStateObject );
        void BindDepthStencilStateCmdImpl( NativeCommandList* nativeCommandList, NativeDepthStencilStateObject* depthStencilStateObject );
    }
}
#endif
