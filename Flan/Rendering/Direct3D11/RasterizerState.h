/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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
#include <Rendering/RasterizerState.h>

#include <d3d11.h>

struct NativeRenderContext;
struct NativeCommandList;

struct NativeRasterizerStateObject
{
    ID3D11RasterizerState* rasterizerStateObject;
    
    NativeRasterizerStateObject()
        : rasterizerStateObject( nullptr )
    {

    }
};

namespace flan
{
    namespace rendering
    {
        NativeRasterizerStateObject* CreateRasterizerStateImpl( NativeRenderContext* nativeRenderContext, const RasterizerStateDesc& description );
        void DestroyRasterizerStateImpl( NativeRenderContext* nativeRenderContext, NativeRasterizerStateObject* RasterizerStateObject );
        void BindRasterizerStateCmdImpl( NativeCommandList* nativeCmdList, NativeRasterizerStateObject* RasterizerStateObject );
    }
}
#endif
