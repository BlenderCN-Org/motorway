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

class RenderDevice;
class CommandList;
class Buffer;
struct NativeVertexArrayObject;

#include <vector>
#include "ImageFormat.h"

struct VertexLayoutEntry
{
    uint32_t        index;

    enum : uint32_t
    {
        DIMENSION_X = 1,
        DIMENSION_XY = 2,
        DIMENSION_XYZ = 3,
        DIMENSION_XYZW = 4,
    } dimension;

    enum : uint32_t
    {
        FORMAT_BYTE,
        FORMAT_SHORT,
        FORMAT_INT,
        FORMAT_FLOAT,
        FORMAT_HALF_FLOAT,
        FORMAT_DOUBLE,
    } format;

    uint32_t    offset;
};

using VertexLayout_t = std::vector<VertexLayoutEntry>;

class VertexArrayObject
{
public:
                                VertexArrayObject();
                                VertexArrayObject( VertexArrayObject& ) = default;
                                VertexArrayObject& operator = ( VertexArrayObject& ) = default;
                                ~VertexArrayObject();

    void                        create( RenderDevice* renderDevice, Buffer* vbo, Buffer* ibo = nullptr );
    void                        destroy( RenderDevice* renderDevice );
    void                        setVertexLayout( RenderDevice* renderDevice, const VertexLayout_t& vertexLayout );

    void                        bind( CommandList* cmdList ) const;

private:
    std::unique_ptr<NativeVertexArrayObject>    nativeVertexArrayObject;
    Buffer*                                     vertexBuffer;
    Buffer*                                     indiceBuffer;
};
