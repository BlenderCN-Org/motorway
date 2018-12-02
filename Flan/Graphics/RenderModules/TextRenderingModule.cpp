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

#include "Shared.h"
#include "TextRenderingModule.h"

#include <Core/Profiler.h>
#include <Core/Factory.h>

#include <Rendering/CommandList.h>

TextRenderingModule::TextRenderingModule()
    : fontAtlas( nullptr )
    , fontDescriptor( nullptr )
    , indiceCount( 0 )
    , vertexBufferIndex( 0 )
    , glyphIndiceBuffer( new Buffer() )
    , bufferIndex( 0 )
    , buffer{ 0.0f }
{
    for ( int32_t i = 0; i < BUFFER_COUNT; i++ ) {
        glyphVertexBuffers[i].reset( new Buffer() );
        glyphVAO[i].reset( new VertexArrayObject() );
    }
}

TextRenderingModule::~TextRenderingModule()
{
    fontAtlas = nullptr;
    fontDescriptor = nullptr;

    indiceCount = 0;
    vertexBufferIndex = 0;

    bufferIndex = 0;
}

void TextRenderingModule::destroy( RenderDevice* renderDevice )
{
    for ( int32_t i = 0; i < BUFFER_COUNT; i++ ) {
        glyphVAO[i]->destroy( renderDevice );
        glyphVertexBuffers[i]->destroy( renderDevice );
    }

    glyphIndiceBuffer->destroy( renderDevice );
}

fnPipelineMutableResHandle_t TextRenderingModule::addTextRenderPass( RenderPipeline* renderPipeline )
{
    using namespace flan::rendering;

    auto data = renderPipeline->addRenderPass(
        "Text Rendering Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            passData.output[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainPresentColorRT" ) );

            RenderPassPipelineStateDesc pipelineState = {};
            pipelineState.hashcode = FLAN_STRING_HASH( "TextRendering" );

            pipelineState.vertexStage = FLAN_STRING( "SDFTextRendering" );
            pipelineState.pixelStage = FLAN_STRING( "SDFTextRendering" );

            pipelineState.primitiveTopology = ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

            pipelineState.blendState.enableBlend = true;
            pipelineState.blendState.sampleMask = ~0;

            pipelineState.blendState.blendConfColor.operation = eBlendOperation::BLEND_OPERATION_ADD;
            pipelineState.blendState.blendConfColor.source = eBlendSource::BLEND_SOURCE_SRC_ALPHA;
            pipelineState.blendState.blendConfColor.dest = eBlendSource::BLEND_SOURCE_INV_SRC_ALPHA;

            pipelineState.blendState.blendConfAlpha.operation = eBlendOperation::BLEND_OPERATION_ADD;
            pipelineState.blendState.blendConfAlpha.source = eBlendSource::BLEND_SOURCE_INV_DEST_ALPHA;
            pipelineState.blendState.blendConfAlpha.dest = eBlendSource::BLEND_SOURCE_ONE;

            pipelineState.rasterizerState.cullMode = eCullMode::CULL_MODE_NONE;
            pipelineState.rasterizerState.fillMode = eFillMode::FILL_MODE_SOLID;
            pipelineState.rasterizerState.useTriangleCCW = false;

            pipelineState.depthStencilState.enableDepthTest = false;
            pipelineState.depthStencilState.enableDepthWrite = false;
            pipelineState.depthStencilState.depthComparisonFunc = eComparisonFunction::COMPARISON_FUNCTION_ALWAYS;

            pipelineState.inputLayout = {
                { 0, IMAGE_FORMAT_R32G32B32A32_FLOAT, 0, 0, 0, true, "POSITION" },
                { 0, IMAGE_FORMAT_R32G32_FLOAT, 0, 0,  0, true, "TEXCOORD" },
                { 0, IMAGE_FORMAT_R32G32B32A32_FLOAT, 0, 0, 0, true, "COLOR" },
            };

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( pipelineState );

            BufferDesc passBuffer;
            passBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            passBuffer.Size = sizeof( glm::uvec4 );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( passBuffer );

            // Sampler State
            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = flan::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.samplers[0] = renderPipelineBuilder->allocateSampler( bilinearSamplerDesc );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            // Bind Font atlas
            fontAtlas->bind( cmdList, 2 );

            // Bind Sampler
            auto bilinearSampler = renderPipelineResources->getSampler( passData.samplers[0] );
            bilinearSampler->bind( cmdList, 2 );

            // Bind RenderTarget
            auto renderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &renderTarget );

            // Bind buffer
            glm::uvec2 rtDimensions;

            auto& pipelineDimensions = renderPipelineResources->getActiveViewport();

            rtDimensions.x = static_cast<uint32_t>( pipelineDimensions.Width );
            rtDimensions.y = static_cast<uint32_t>( pipelineDimensions.Height );

            auto bufferData = renderPipelineResources->getBuffer( passData.buffers[0] );
            bufferData->updateAsynchronous( cmdList, &rtDimensions, sizeof( glm::uvec2 ) );
            bufferData->bind( cmdList, 0u, SHADER_STAGE_VERTEX );

            // Update viewport
            cmdList->setViewportCmd( pipelineDimensions );

            // Bind Vertex array
            glyphVertexBuffers[vertexBufferIndex]->updateAsynchronous( cmdList, buffer, bufferIndex * sizeof( float ) );

            glyphVAO[vertexBufferIndex]->bind( cmdList );

            cmdList->drawIndexedCmd( static_cast<uint32_t>( indiceCount ) );

            cmdList->bindBackbufferCmd();
            fontAtlas->unbind( cmdList );

            // Swap buffers
            vertexBufferIndex = ( vertexBufferIndex == 0 ) ? 1 : 0;

            // Reset buffers
            indiceCount = 0;
            bufferIndex = 0;
        } );

    return data.output[0];
}

void TextRenderingModule::loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager )
{
    // Load Default Font
    fontDescriptor = graphicsAssetManager->getFont( FLAN_STRING( "GameData/Fonts/SegoeUI.fnt" ) );

    if ( fontDescriptor == nullptr ) {
        FLAN_CERR << "Could not load default font descriptor; stopping here" << std::endl;
        return;
    }

    fontAtlas = graphicsAssetManager->getTexture( fontDescriptor->Name.c_str() );

    // Create static indice buffer
    static constexpr int IndexStride = sizeof( uint32_t );

    // Indices per glyph (2 triangles; 3 indices per triangle)
    static constexpr int indexBufferSize = MaxCharactersPerLine * MaxCharactersLines * 6 * sizeof( uint32_t );
    static constexpr int indexBufferLength = indexBufferSize / IndexStride;

    uint32_t indexBufferData[indexBufferLength];

    uint32_t i = 0;
    for ( uint32_t c = 0; c < MaxCharactersPerLine * MaxCharactersLines; c++ ) {
        indexBufferData[i + 0] = c * 4 + 0;
        indexBufferData[i + 1] = c * 4 + 1;
        indexBufferData[i + 2] = c * 4 + 2;

        indexBufferData[i + 3] = c * 4 + 0;
        indexBufferData[i + 4] = c * 4 + 2;
        indexBufferData[i + 5] = c * 4 + 3;

        i += 6;
    }

    BufferDesc indiceBufferDescription;
    indiceBufferDescription.Type = BufferDesc::INDICE_BUFFER;
    indiceBufferDescription.Size = indexBufferSize;

    glyphIndiceBuffer->create( renderDevice, indiceBufferDescription, indexBufferData );

    // Create dynamic vertex buffer
    // 4 + 2 + 4
    constexpr std::size_t SINGLE_VERTEX_SIZE = sizeof( float ) * 10;

    VertexLayout_t glyphLayout = {
        { 0, VertexLayoutEntry::DIMENSION_XYZW, VertexLayoutEntry::FORMAT_FLOAT, 0 },
        { 1, VertexLayoutEntry::DIMENSION_XY, VertexLayoutEntry::FORMAT_FLOAT, 4 * sizeof( float ) },
        { 2, VertexLayoutEntry::DIMENSION_XYZW, VertexLayoutEntry::FORMAT_FLOAT, 6 * sizeof( float ) },
    };

    // XYZ Position + RGBA Color + XY TexCoords (w/ 6 vertices per glyph)
    constexpr std::size_t VERTEX_BUFFER_SIZE = SINGLE_VERTEX_SIZE * TEXT_RENDERING_MAX_GLYPH_COUNT;

    BufferDesc bufferDescription;
    bufferDescription.Type = BufferDesc::DYNAMIC_VERTEX_BUFFER;
    bufferDescription.Size = VERTEX_BUFFER_SIZE;
    bufferDescription.Stride = SINGLE_VERTEX_SIZE;

    for ( int32_t i = 0; i < BUFFER_COUNT; i++ ) {
        glyphVertexBuffers[i]->create( renderDevice, bufferDescription );

        // Create VAO and set its layout
        glyphVAO[i]->create( renderDevice, glyphVertexBuffers[i].get(), glyphIndiceBuffer.get() );
        glyphVAO[i]->setVertexLayout( renderDevice, glyphLayout );
    }

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( FLAN_STRING_HASH( "TextRenderPass" ), [=]( RenderPipeline* renderPipeline ) { return addTextRenderPass( renderPipeline ); } );
}

void TextRenderingModule::addOutlinedText( const char* text, float size, float x, float y, const glm::vec4& textColor, const float outlineThickness )
{
    const float baseX = x, localSize = size;
    float localX = x, localY = y;

    const auto localTextColor = textColor;

    int charIdx = 0;
    for ( const char* p = text; *p != '\0'; p++, charIdx++ ) {
        auto g = fontDescriptor->Glyphes[*p];

        float gx = localX + static_cast<float>( g.OffsetX ) * localSize;
        float gy = -localY - static_cast<float>( g.OffsetY ) * localSize;
        float gw = static_cast<float>( g.Width ) * localSize;
        float gh = static_cast<float>( g.Height ) * localSize;

        localX += g.AdvanceX * localSize;

        if ( *p == '\n' ) {
            localY += 38 * localSize;
            localX = baseX;
            charIdx = 0;
            continue;
        } else if ( *p == '\t' ) {
            localX += 28 * localSize * ( ( charIdx + 1 ) % 4 );
            continue;
        }

        if ( gw <= 0.0f || gh <= 0.0f )
            continue;

        float u1 = static_cast<float>( g.PositionX ) / static_cast<float>( fontDescriptor->AtlasWidth );
        float u2 = u1 + ( static_cast<float>( g.Width ) / static_cast<float>( fontDescriptor->AtlasWidth ) );
        float v1 = static_cast<float>( g.PositionY ) / static_cast<float>( fontDescriptor->AtlasHeight );
        float v2 = v1 + ( static_cast<float>( g.Height ) / static_cast<float>( fontDescriptor->AtlasHeight ) );

        buffer[bufferIndex++] = ( gx );
        buffer[bufferIndex++] = ( gy );
        buffer[bufferIndex++] = ( 1.0f );
        buffer[bufferIndex++] = ( outlineThickness );
        buffer[bufferIndex++] = ( u1 );
        buffer[bufferIndex++] = ( v1 );
        buffer[bufferIndex++] = ( localTextColor.r );
        buffer[bufferIndex++] = ( localTextColor.g );
        buffer[bufferIndex++] = ( localTextColor.b );
        buffer[bufferIndex++] = ( localTextColor.a );

        buffer[bufferIndex++] = ( gx );
        buffer[bufferIndex++] = ( gy - gh );
        buffer[bufferIndex++] = ( 1.0f );
        buffer[bufferIndex++] = ( outlineThickness );
        buffer[bufferIndex++] = ( u1 );
        buffer[bufferIndex++] = ( v2 );
        buffer[bufferIndex++] = ( localTextColor.r );
        buffer[bufferIndex++] = ( localTextColor.g );
        buffer[bufferIndex++] = ( localTextColor.b );
        buffer[bufferIndex++] = ( localTextColor.a );

        buffer[bufferIndex++] = ( gx + gw );
        buffer[bufferIndex++] = ( gy - gh );
        buffer[bufferIndex++] = ( 1.0f );
        buffer[bufferIndex++] = ( outlineThickness );
        buffer[bufferIndex++] = ( u2 );
        buffer[bufferIndex++] = ( v2 );
        buffer[bufferIndex++] = ( localTextColor.r );
        buffer[bufferIndex++] = ( localTextColor.g );
        buffer[bufferIndex++] = ( localTextColor.b );
        buffer[bufferIndex++] = ( localTextColor.a );

        buffer[bufferIndex++] = ( gx + gw );
        buffer[bufferIndex++] = ( gy );
        buffer[bufferIndex++] = ( 1.0f );
        buffer[bufferIndex++] = ( outlineThickness );
        buffer[bufferIndex++] = ( u2 );
        buffer[bufferIndex++] = ( v1 );
        buffer[bufferIndex++] = ( localTextColor.r );
        buffer[bufferIndex++] = ( localTextColor.g );
        buffer[bufferIndex++] = ( localTextColor.b );
        buffer[bufferIndex++] = ( localTextColor.a );

        indiceCount += 6;
    }
}
