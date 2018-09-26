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
#include "VertexArrayObject.h"

#include "RenderDevice.h"
#include "CommandList.h"

#include "Buffer.h"

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#include "OpenGL460/VertexArrayObject.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#include "Direct3D11/CommandList.h"
#include "Direct3D11/VertexArrayObject.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#include "Vulkan/CommandList.h"
#include "Vulkan/VertexArrayObject.h"
#endif

VertexArrayObject::VertexArrayObject()
    : nativeVertexArrayObject( nullptr )
    , vertexBuffer( nullptr )
    , indiceBuffer( nullptr )
{

}

VertexArrayObject::~VertexArrayObject()
{

}

void VertexArrayObject::create( RenderDevice* renderDevice, Buffer* vbo, Buffer* ibo )
{
    vertexBuffer = vbo;
    indiceBuffer = ibo;

    nativeVertexArrayObject.reset( flan::rendering::CreateVertexArrayImpl( renderDevice->getNativeRenderContext(), vertexBuffer, indiceBuffer ) );
}

void VertexArrayObject::destroy( RenderDevice* renderDevice )
{
    vertexBuffer->destroy( renderDevice );
    indiceBuffer->destroy( renderDevice );

    flan::rendering::DestroyVertexArrayImpl( renderDevice->getNativeRenderContext(), nativeVertexArrayObject.get() );
}

void VertexArrayObject::setVertexLayout( RenderDevice* renderDevice, const VertexLayout_t& vertexLayout )
{
    flan::rendering::SetVertexArrayVertexLayoutImpl( renderDevice->getNativeRenderContext(), nativeVertexArrayObject.get(), vertexLayout );
}

void VertexArrayObject::bind( CommandList* cmdList ) const
{
    flan::rendering::BindVertexArrayCmdImpl( cmdList->getNativeCommandList(), nativeVertexArrayObject.get() );

    vertexBuffer->bind( cmdList );

    if ( indiceBuffer != nullptr ) {
        indiceBuffer->bind( cmdList );
    }
}
