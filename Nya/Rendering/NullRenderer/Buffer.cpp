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
#include <Shared.h>

#if NYA_NULL_RENDERER
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

Buffer* RenderDevice::createBuffer( const BufferDesc& description, const void* initialData )
{
    return nullptr;
}

void RenderDevice::destroyBuffer( Buffer* buffer )
{

}

void RenderDevice::setDebugMarker( Buffer* buffer, const char* objectName )
{

}

void CommandList::bindVertexBuffer( const Buffer* buffer, const unsigned int bindIndex )
{

}

void CommandList::bindIndiceBuffer( const Buffer* buffer )
{

}

void CommandList::updateBuffer( Buffer* buffer, const void* data, const size_t dataSize )
{

}

void CommandList::copyStructureCount( Buffer* srcBuffer, Buffer* dstBuffer, const unsigned int offset )
{

}

void CommandList::drawInstancedIndirect( const Buffer* drawArgsBuffer, const unsigned int bufferDataOffset )
{

}
#endif
