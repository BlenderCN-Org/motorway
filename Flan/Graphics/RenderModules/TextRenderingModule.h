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
#include <Io/FontDescriptor.h>

static constexpr int32_t MaxCharactersPerLine = 240;
static constexpr int32_t MaxCharactersLines = 65;

static constexpr int32_t TEXT_RENDERING_MAX_GLYPH_COUNT = MaxCharactersPerLine * MaxCharactersLines * 4;

class TextRenderingModule
{
public:
                                TextRenderingModule();
                                TextRenderingModule( TextRenderingModule& ) = delete;
                                ~TextRenderingModule();

    void                         destroy( RenderDevice* renderDevice );
    fnPipelineMutableResHandle_t addTextRenderPass( RenderPipeline* renderPipeline );
    void                         loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager );

    void                         addOutlinedText( const char* text, float size, float x, float y, const glm::vec4& textColor = glm::vec4( 1, 1, 1, 1 ), const float outlineThickness = 0.80f );

private:
    static constexpr int BUFFER_COUNT = 2;

private:
    Texture*                    fontAtlas;
    FontDescriptor*             fontDescriptor;

    int                         indiceCount;
    int                         vertexBufferIndex;

    std::unique_ptr<Buffer>     glyphVertexBuffers[BUFFER_COUNT];
    std::unique_ptr<Buffer>     glyphIndiceBuffer;

    std::unique_ptr<VertexArrayObject> glyphVAO[BUFFER_COUNT];

    int                         bufferIndex;
    float                       buffer[9 * TEXT_RENDERING_MAX_GLYPH_COUNT];
};
