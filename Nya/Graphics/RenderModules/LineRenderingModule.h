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

#include <Maths/Vector.h>

struct PipelineState;
struct Buffer;

class RenderPipeline;
class RenderDevice;
class GraphicsAssetCache;
class ShaderCache;

using ResHandle_t = uint32_t;

static constexpr int32_t LINE_RENDERING_MAX_LINE_COUNT = 32000;

class LineRenderingModule
{
public:
                                LineRenderingModule();
                                LineRenderingModule( LineRenderingModule& ) = delete;
                                ~LineRenderingModule();

    void                         destroy( RenderDevice* renderDevice );
    ResHandle_t                  addLineRenderPass( RenderPipeline* renderPipeline, ResHandle_t output );
    void                         loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache );

    void                         addLine( const nyaVec3f& from, const nyaVec3f& to, const float thickness, const nyaVec4f& color );

private:
    static constexpr int BUFFER_COUNT = 2;

private:
    PipelineState* renderLinePso;

    int         indiceCount;
    int         vertexBufferIndex;

    Buffer*     lineVertexBuffers[BUFFER_COUNT];
    Buffer*     lineIndiceBuffer;

    int         bufferIndex;
    float       buffer[LINE_RENDERING_MAX_LINE_COUNT];
};