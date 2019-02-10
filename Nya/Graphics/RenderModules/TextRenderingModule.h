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

struct Texture;
struct FontDescriptor;
struct PipelineState;
struct Buffer;

class RenderPipeline;
class RenderDevice;
class GraphicsAssetCache;
class ShaderCache;

using MutableResHandle_t = uint32_t;

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
    MutableResHandle_t           renderText( RenderPipeline* renderPipeline, MutableResHandle_t output );
    void                         loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache );

    void                         addOutlinedText( const char* text, float size, float x, float y, const nyaVec4f& textColor = nyaVec4f( 1, 1, 1, 1 ), const float outlineThickness = 0.80f );

private:
    static constexpr int BUFFER_COUNT = 2;

private:
    Texture*                    fontAtlas;
    FontDescriptor*             fontDescriptor;
    PipelineState*              renderTextPso;

    uint32_t                    indiceCount;
    uint32_t                    vertexBufferIndex;
    Buffer*                     glyphVertexBuffers[BUFFER_COUNT];
    Buffer*                     glyphIndiceBuffer;

    int                         bufferOffset;
    float                       buffer[9 * TEXT_RENDERING_MAX_GLYPH_COUNT];
};
