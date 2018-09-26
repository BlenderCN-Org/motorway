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
#include <Rendering/Buffer.h>
#include <Rendering/VertexArrayObject.h>

struct NativeRenderContext;
struct NativeCommandList;
class Buffer;

struct NativeVertexArrayObject
{

};

namespace flan
{
    namespace rendering
    {
        NativeVertexArrayObject* CreateVertexArrayImpl( NativeRenderContext* nativeRenderContext, Buffer* vbo, Buffer* ibo );
        void DestroyVertexArrayImpl( NativeRenderContext* nativeRenderContext, NativeVertexArrayObject* vertexArrayObject );
        void SetVertexArrayVertexLayoutImpl( NativeRenderContext* nativeRenderContext, NativeVertexArrayObject* vertexArrayObject, const VertexLayout_t& vertexLayout );
        void BindVertexArrayCmdImpl( NativeCommandList* nativeCmdList, NativeVertexArrayObject* vertexArrayObject );
    }
}
#endif
