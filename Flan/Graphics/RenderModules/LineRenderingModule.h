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

#include <Rendering/RenderDevice.h>
#include <Rendering/Buffer.h>
#include <Rendering/PipelineState.h>
#include <Rendering/VertexArrayObject.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/RenderPipeline.h>

static constexpr int32_t LINE_RENDERING_MAX_LINE_COUNT = 32000;

class LineRenderingModule
{
public:
                                LineRenderingModule();
                                LineRenderingModule( LineRenderingModule& ) = delete;
                                ~LineRenderingModule();

    void                         destroy( RenderDevice* renderDevice );
    fnPipelineMutableResHandle_t addLineRenderPass( RenderPipeline* renderPipeline );
    void                         loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager );

    void                         addLine( const glm::vec3& from, const glm::vec3& to, const float thickness, const glm::vec4& color );

private:
    static constexpr int BUFFER_COUNT = 2;

private:
    int                         indiceCount;
    int                         vertexBufferIndex;

    std::unique_ptr<Buffer>     lineVertexBuffers[BUFFER_COUNT];
    std::unique_ptr<Buffer>     lineIndiceBuffer;

    std::unique_ptr<VertexArrayObject> lineVAO[BUFFER_COUNT];

    int                         bufferIndex;
    float                       buffer[LINE_RENDERING_MAX_LINE_COUNT];
};
