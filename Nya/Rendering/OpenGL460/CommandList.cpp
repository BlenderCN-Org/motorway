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

#if NYA_GL460
#include <Rendering/CommandList.h>
#include <Rendering/RenderDevice.h>

#include "Extensions.h"

using namespace nya::rendering;

static constexpr GLenum GL_PRIMITIVE_TOPOLOGY[ePrimitiveTopology::PRIMITIVE_TOPOLOGY_COUNT] = {
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_LINE
};

GLenum GetGLIndiceType( const std::size_t singleIndiceSize )
{
    GLenum glIndiceType = GL_UNSIGNED_INT;
    if ( singleIndiceSize == 1 ) {
        glIndiceType = GL_UNSIGNED_BYTE;
    } else if ( singleIndiceSize == 2 ) {
        glIndiceType = GL_UNSIGNED_SHORT;
    }

    return glIndiceType;
}

struct NativeCommandList
{
    GLenum  primitiveTopology;
};

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
    glDrawArrays( CommandListObject->primitiveTopology, static_cast<GLint>( vertexOffset ), static_cast<GLsizei>( vertexCount ) );
}

void CommandList::drawIndexed( const unsigned int indiceCount, const unsigned int indiceOffset, const size_t indiceType, const unsigned int vertexOffset )
{
    glDrawElementsBaseVertex( CommandListObject->primitiveTopology, static_cast<GLsizei>( indiceCount ), GetGLIndiceType( indiceType ), static_cast<const char*>( nullptr ) + indiceOffset * indiceType, static_cast<GLint>( vertexOffset ) );
}

void CommandList::drawInstancedIndexed( const unsigned int indiceCount, const unsigned int instanceCount, const unsigned int indiceOffset, const unsigned int vertexOffset, const unsigned int instanceOffset )
{
    glDrawArraysInstanced( CommandListObject->primitiveTopology, indiceOffset, indiceCount, instanceCount );
}

void CommandList::dispatchCompute( const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ )
{
    glDispatchCompute( threadCountX, threadCountY, threadCountZ );
    glMemoryBarrier( GL_ALL_BARRIER_BITS );
}

void CommandList::setViewport( const Viewport& viewport )
{
    glViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    glDepthRange( static_cast<GLclampd>( viewport.MinDepth ), static_cast<GLclampd>( viewport.MaxDepth ) );
}

void CommandList::getViewport( Viewport& viewport )
{
    GLint v[4];
    glGetIntegerv( GL_VIEWPORT, v );

    viewport.X = v[0];
    viewport.Y = v[1];

    viewport.Width = v[2];
    viewport.Height = v[3];

    GLfloat depthRange[2];
    glGetFloatv( GL_DEPTH_RANGE, depthRange );

    viewport.MinDepth = depthRange[0];
    viewport.MaxDepth = depthRange[1];
}
#endif
