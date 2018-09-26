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
#include "LineRenderingModule.h"

#include <Core/Profiler.h>
#include <Core/Factory.h>

#include <Rendering/CommandList.h>

LineRenderingModule::LineRenderingModule()
    : indiceCount( 0 )
    , vertexBufferIndex( 0 )
    , lineIndiceBuffer( new Buffer() )
    , bufferIndex( 0 )
    , buffer{ 0.0f }
{
    memset( buffer, 0, sizeof( float ) * LINE_RENDERING_MAX_LINE_COUNT );
    for ( int32_t i = 0; i < BUFFER_COUNT; i++ ) {
        lineVertexBuffers[i].reset( new Buffer() );
        lineVAO[i].reset( new VertexArrayObject() );
    }
}

LineRenderingModule::~LineRenderingModule()
{
    indiceCount = 0;
    vertexBufferIndex = 0;

    bufferIndex = 0;
}

void LineRenderingModule::destroy( RenderDevice* renderDevice )
{
    for ( int32_t i = 0; i < BUFFER_COUNT; i++ ) {
        lineVAO[i]->destroy( renderDevice );
        lineVertexBuffers[i]->destroy( renderDevice );
    }

    lineIndiceBuffer->destroy( renderDevice );
}

fnPipelineMutableResHandle_t LineRenderingModule::addLineRenderPass( RenderPipeline* renderPipeline )
{
    using namespace flan::rendering;

    auto data = renderPipeline->addRenderPass(
        "Line Rendering Pass",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            passData.output[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );
            passData.input[0] = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainDepthRT" ) );

            RenderPassPipelineStateDesc pipelineState = {};
            pipelineState.hashcode = FLAN_STRING_HASH( "LineRendering" );

            pipelineState.vertexStage = FLAN_STRING( "Line" );
            pipelineState.pixelStage = FLAN_STRING( "Line" );

            pipelineState.primitiveTopology = ePrimitiveTopology::PRIMITIVE_TOPOLOGY_LINELIST;

            pipelineState.rasterizerState.fillMode = eFillMode::FILL_MODE_WIREFRAME;

            pipelineState.rasterizerState.cullMode = eCullMode::CULL_MODE_BACK;
            pipelineState.rasterizerState.useTriangleCCW = false;

            pipelineState.depthStencilState.enableDepthTest = false;
            pipelineState.depthStencilState.enableDepthWrite = false;
            pipelineState.depthStencilState.depthComparisonFunc = eComparisonFunction::COMPARISON_FUNCTION_ALWAYS;

            pipelineState.inputLayout = {
                { 0, IMAGE_FORMAT_R32G32B32A32_FLOAT, 0, 0, 0, true, "POSITION" },
                { 0, IMAGE_FORMAT_R32G32B32A32_FLOAT, 0, 0, 0, true, "COLOR" },
            };

            passData.pipelineState = renderPipelineBuilder->allocatePipelineState( pipelineState );

            BufferDesc cameraBuffer = {};
            cameraBuffer.Type = BufferDesc::CONSTANT_BUFFER;
            cameraBuffer.Size = sizeof( Camera::Data );

            passData.buffers[0] = renderPipelineBuilder->allocateBuffer( cameraBuffer );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Bind Pass Pipeline State
            auto pipelineState = renderPipelineResources->getPipelineState( passData.pipelineState );
            cmdList->bindPipelineStateCmd( pipelineState );

            auto depthBuffer = renderPipelineResources->getRenderTarget( passData.input[0] );

            // Bind RenderTarget
            auto renderTarget = renderPipelineResources->getRenderTarget( passData.output[0] );
            cmdList->bindRenderTargetsCmd( &renderTarget, depthBuffer );

            // Bind Camera Buffer
            auto cameraCbuffer = renderPipelineResources->getBuffer( passData.buffers[0] );
            auto passCamera = renderPipelineResources->getActiveCamera();
            cameraCbuffer->updateAsynchronous( cmdList, &passCamera, sizeof( Camera::Data ) );
            cameraCbuffer->bind( cmdList, 0 );

            // Update viewport
            auto& pipelineDimensions = renderPipelineResources->getActiveViewport();
            cmdList->setViewportCmd( pipelineDimensions );

            // Bind Vertex array
            lineVertexBuffers[vertexBufferIndex]->updateAsynchronous( cmdList, buffer, bufferIndex * sizeof( float ) );

            lineVAO[vertexBufferIndex]->bind( cmdList );

            cmdList->drawCmd( indiceCount );

            cmdList->bindBackbufferCmd();

            // Swap buffers
            vertexBufferIndex = ( vertexBufferIndex == 0 ) ? 1 : 0;

            // Reset buffers
            indiceCount = 0;
            bufferIndex = 0;
        } );

    return data.output[0];
}

void LineRenderingModule::loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager )
{
    // Create static indice buffer
    static constexpr int IndexStride = sizeof( uint32_t );

    // Indices per line (2 triangles; 3 indices per triangle)
    static constexpr int indexBufferSize = LINE_RENDERING_MAX_LINE_COUNT * 2 * sizeof( uint32_t );
    static constexpr int indexBufferLength = indexBufferSize / IndexStride;

    uint32_t indexBufferData[indexBufferLength];

    uint32_t i = 0;
    for ( uint32_t c = 0; c < LINE_RENDERING_MAX_LINE_COUNT; c++ ) {
        indexBufferData[c * 2] = c;
        indexBufferData[( c * 2 ) + 1] = ( c + 1 );
    }

    BufferDesc indiceBufferDescription;
    indiceBufferDescription.Type = BufferDesc::INDICE_BUFFER;
    indiceBufferDescription.Size = indexBufferSize;

    lineIndiceBuffer->create( renderDevice, indiceBufferDescription, indexBufferData );

    // Create dynamic vertex buffer
    // 4 + 4
    constexpr std::size_t SINGLE_VERTEX_SIZE = sizeof( float ) * 8;

    VertexLayout_t lineLayout = {
        { 0, VertexLayoutEntry::DIMENSION_XYZW, VertexLayoutEntry::FORMAT_FLOAT, 0 },
        { 1, VertexLayoutEntry::DIMENSION_XYZW, VertexLayoutEntry::FORMAT_FLOAT, 4 * sizeof( float ) },
    };

    // XYZW Position + RGBA Color (w/ 2 vertices per line)
    constexpr std::size_t VERTEX_BUFFER_SIZE = SINGLE_VERTEX_SIZE * LINE_RENDERING_MAX_LINE_COUNT * 2;

    BufferDesc bufferDescription;
    bufferDescription.Type = BufferDesc::DYNAMIC_VERTEX_BUFFER;
    bufferDescription.Size = VERTEX_BUFFER_SIZE;
    bufferDescription.Stride = SINGLE_VERTEX_SIZE;

    for ( int32_t i = 0; i < BUFFER_COUNT; i++ ) {
        lineVertexBuffers[i]->create( renderDevice, bufferDescription );

        // Create VAO and set its layout
        lineVAO[i]->create( renderDevice, lineVertexBuffers[i].get(), lineIndiceBuffer.get() );
        lineVAO[i]->setVertexLayout( renderDevice, lineLayout );
    }

    Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( FLAN_STRING_HASH( "LineRenderPass" ), [=]( RenderPipeline* renderPipeline ) { return addLineRenderPass( renderPipeline ); } );
}

void LineRenderingModule::addLine( const glm::vec3& from, const glm::vec3& to, const float thickness, const glm::vec4& color )
{
    buffer[bufferIndex++] = ( from.x );
    buffer[bufferIndex++] = ( from.y );
    buffer[bufferIndex++] = ( from.z );
    buffer[bufferIndex++] = ( thickness );

    buffer[bufferIndex++] = ( color.r );
    buffer[bufferIndex++] = ( color.g );
    buffer[bufferIndex++] = ( color.b );
    buffer[bufferIndex++] = ( color.a );

    buffer[bufferIndex++] = ( to.x );
    buffer[bufferIndex++] = ( to.y );
    buffer[bufferIndex++] = ( to.z );
    buffer[bufferIndex++] = ( thickness );

    buffer[bufferIndex++] = ( color.r );
    buffer[bufferIndex++] = ( color.g );
    buffer[bufferIndex++] = ( color.b );
    buffer[bufferIndex++] = ( color.a );

    indiceCount += 2;
}
