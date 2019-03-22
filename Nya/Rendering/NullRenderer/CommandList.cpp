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
#include <Rendering/CommandList.h>

CommandList::~CommandList()
{

}

void CommandList::begin()
{

}

void CommandList::end()
{

}

void CommandList::draw( const unsigned int vertexCount, const unsigned int vertexOffset )
{

}

void CommandList::drawIndexed( const unsigned int indiceCount, const unsigned int indiceOffset, const size_t indiceType, const unsigned int vertexOffset )
{

}

void CommandList::drawInstancedIndexed( const unsigned int indiceCount, const unsigned int instanceCount, const unsigned int indiceOffset, const unsigned int vertexOffset, const unsigned int instanceOffset )
{

}

void CommandList::dispatchCompute( const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ )
{

}

void CommandList::setViewport( const Viewport& viewport )
{

}

void CommandList::getViewport( Viewport& viewport )
{

}
#endif
