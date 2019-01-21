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

#if NYA_D3D11
#include <Rendering/CommandList.h>
#include "CommandList.h"

#include <d3d11.h>

CommandList::~CommandList()
{
    NativeCommandList->deferredContext->Release();
}

void CommandList::begin()
{

}

void CommandList::end()
{
    NativeCommandList->deferredContext->FinishCommandList( FALSE, &NativeCommandList->commandList );
}

void CommandList::draw( const unsigned int vertexCount, const unsigned int vertexOffset )
{
    NativeCommandList->deferredContext->Draw( vertexCount, vertexOffset );
}

void CommandList::drawIndexed( const unsigned int indiceCount, const unsigned int indiceOffset, const size_t indiceType, const unsigned int vertexOffset )
{
    NativeCommandList->deferredContext->DrawIndexed( indiceCount, indiceOffset, vertexOffset );
}

void CommandList::drawInstancedIndexed( const unsigned int indiceCount, const unsigned int instanceCount, const unsigned int indiceOffset, const unsigned int vertexOffset, const unsigned int instanceOffset )
{
    NativeCommandList->deferredContext->DrawIndexedInstanced( indiceCount, instanceCount, indiceOffset, vertexOffset, instanceOffset );
}

void CommandList::dispatchCompute( const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ )
{
    NativeCommandList->deferredContext->Dispatch( threadCountX, threadCountY, threadCountZ );
}

void CommandList::setViewport( const Viewport& viewport )
{
    const D3D11_VIEWPORT d3dViewport =
    {
        static_cast<FLOAT>( viewport.X ),
        static_cast<FLOAT>( viewport.Y ),
        static_cast<FLOAT>( viewport.Width ),
        static_cast<FLOAT>( viewport.Height ),
        viewport.MinDepth,
        viewport.MaxDepth,
    };

    NativeCommandList->deferredContext->RSSetViewports( 1, &d3dViewport );
}

void CommandList::getViewport( Viewport& viewport )
{
    // TODO Support multiple viewport (if it comes useful at some point)
    UINT viewportCount = 1;

    D3D11_VIEWPORT activeViewport = {};
    NativeCommandList->deferredContext->RSGetViewports( &viewportCount, &activeViewport );

    viewport.X = static_cast<int32_t>( activeViewport.TopLeftX );
    viewport.Y = static_cast<int32_t>( activeViewport.TopLeftY );

    viewport.Width = static_cast<int32_t>( activeViewport.Width );
    viewport.Height = static_cast<int32_t>( activeViewport.Height );

    viewport.MinDepth = activeViewport.MinDepth;
    viewport.MaxDepth = activeViewport.MaxDepth;
}
#endif
