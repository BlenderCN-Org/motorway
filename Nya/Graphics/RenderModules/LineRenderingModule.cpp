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

#include <Graphics/RenderPipeline.h>
#include <Graphics/GraphicsAssetCache.h>
#include <Graphics/ShaderCache.h>

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

#include <Maths/MatrixTransformations.h>

#include <string.h>

using namespace nya::rendering;

LineRenderingModule::LineRenderingModule()
    : indiceCount( 0 )
    , vertexBufferIndex( 0 )
    , lineVertexBuffers{ nullptr, nullptr }
    , lineIndiceBuffer( nullptr )
    , bufferIndex( 0 )
    , buffer{ 0.0f }
{
    memset( buffer, 0, sizeof( float ) * LINE_RENDERING_MAX_LINE_COUNT );
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
        renderDevice->destroyBuffer( lineVertexBuffers[i] );
    }

    renderDevice->destroyBuffer( lineIndiceBuffer );
}

ResHandle_t LineRenderingModule::addLineRenderPass( RenderPipeline* renderPipeline, ResHandle_t output )
{
    struct PassData {
        MutableResHandle_t  output;

        ResHandle_t         screenBuffer;
    };

    PassData& data = renderPipeline->addRenderPass<PassData>(
        "Line Rendering Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
        renderPipelineBuilder.setUncullablePass();

            // Passthrough rendertarget
            passData.output = renderPipelineBuilder.readRenderTarget( output );

            BufferDesc screenBufferDesc;
            screenBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            screenBufferDesc.size = sizeof( nyaMat4x4f );

            passData.screenBuffer = renderPipelineBuilder.allocateBuffer( screenBufferDesc, SHADER_STAGE_VERTEX );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            // Render Pass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );

            RenderPass renderPass;
            renderPass.attachement[0] = { outputTarget, 0, 0 };

            Buffer* screenBuffer = renderPipelineResources.getBuffer( passData.screenBuffer );

            const CameraData* cameraData = renderPipelineResources.getMainCamera();

            Viewport vp;
            vp.X = 0;
            vp.Y = 0;
            vp.Width = static_cast<int>( cameraData->viewportSize.x );
            vp.Height = static_cast<int>( cameraData->viewportSize.y );
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            cmdList->setViewport( vp );

            nyaMat4x4f orthoMatrix = nya::maths::MakeOrtho( 0.0f, cameraData->viewportSize.x, cameraData->viewportSize.y, 0.0f, -1.0f, 1.0f );

            cmdList->updateBuffer( screenBuffer, &orthoMatrix, sizeof( nyaMat4x4f ) );

            ResourceList resourceList;
            resourceList.resource[0].buffer = screenBuffer;

            cmdList->updateBuffer( lineVertexBuffers[vertexBufferIndex], buffer, static_cast<size_t>( bufferIndex ) * sizeof( float ) );         

            // Pipeline State
            cmdList->bindPipelineState( renderLinePso );
            cmdList->bindResourceList( renderLinePso, resourceList );
            cmdList->bindRenderPass( renderLinePso, renderPass );

            // Bind buffers
            cmdList->bindVertexBuffer( lineVertexBuffers[vertexBufferIndex] );
            cmdList->bindIndiceBuffer( lineIndiceBuffer );

            cmdList->draw( static_cast<unsigned int>( indiceCount ) );

            // Swap buffers
            vertexBufferIndex = ( vertexBufferIndex == 0 ) ? 1 : 0;

            // Reset buffers
            indiceCount = 0;
            bufferIndex = 0;
        } 
    );

    return data.output;
}

void LineRenderingModule::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache )
{
    // Create static indice buffer
    static constexpr int IndexStride = sizeof( uint32_t );

    // Indices per line (2 triangles; 3 indices per triangle)
    static constexpr int indexBufferSize = LINE_RENDERING_MAX_LINE_COUNT * 2 * sizeof( uint32_t );
    static constexpr int indexBufferLength = indexBufferSize / IndexStride;

    uint32_t indexBufferData[indexBufferLength];

    for ( uint32_t c = 0; c < LINE_RENDERING_MAX_LINE_COUNT; c++ ) {
        indexBufferData[c * 2] = c;
        indexBufferData[( c * 2 ) + 1] = ( c + 1 );
    }

    BufferDesc indiceBufferDescription;
    indiceBufferDescription.type = BufferDesc::INDICE_BUFFER;
    indiceBufferDescription.size = indexBufferSize;

    lineIndiceBuffer = renderDevice->createBuffer( indiceBufferDescription, indexBufferData );
    
    // Create dynamic vertex buffer
    // 4 + 4
    constexpr std::size_t SINGLE_VERTEX_SIZE = sizeof( float ) * 8;

    // XYZW Position + RGBA Color (w/ 2 vertices per line)
    constexpr std::size_t VERTEX_BUFFER_SIZE = SINGLE_VERTEX_SIZE * LINE_RENDERING_MAX_LINE_COUNT * 2;

    BufferDesc bufferDescription;
    bufferDescription.type = BufferDesc::DYNAMIC_VERTEX_BUFFER;
    bufferDescription.size = VERTEX_BUFFER_SIZE;
    bufferDescription.stride = SINGLE_VERTEX_SIZE;

    for ( int32_t i = 0; i < BUFFER_COUNT; i++ ) {
        lineVertexBuffers[i] = renderDevice->createBuffer( bufferDescription );
    }

    PipelineStateDesc pipelineState = {};
    pipelineState.vertexShader = shaderCache->getOrUploadStage( "LineRendering", eShaderStage::SHADER_STAGE_VERTEX );
    pipelineState.pixelShader = shaderCache->getOrUploadStage( "LineRendering", eShaderStage::SHADER_STAGE_PIXEL );

    pipelineState.primitiveTopology = ePrimitiveTopology::PRIMITIVE_TOPOLOGY_LINELIST;

    pipelineState.blendState.enableBlend = true;
    pipelineState.blendState.sampleMask = ~0u;

    pipelineState.blendState.blendConfColor.operation = eBlendOperation::BLEND_OPERATION_ADD;
    pipelineState.blendState.blendConfColor.source = eBlendSource::BLEND_SOURCE_SRC_ALPHA;
    pipelineState.blendState.blendConfColor.dest = eBlendSource::BLEND_SOURCE_INV_SRC_ALPHA;

    pipelineState.blendState.blendConfAlpha.operation = eBlendOperation::BLEND_OPERATION_ADD;
    pipelineState.blendState.blendConfAlpha.source = eBlendSource::BLEND_SOURCE_INV_DEST_ALPHA;
    pipelineState.blendState.blendConfAlpha.dest = eBlendSource::BLEND_SOURCE_ONE;

    pipelineState.rasterizerState.cullMode = eCullMode::CULL_MODE_BACK;
    pipelineState.rasterizerState.fillMode = eFillMode::FILL_MODE_WIREFRAME;
    pipelineState.rasterizerState.useTriangleCCW = false;

    pipelineState.depthStencilState.enableDepthTest = false;
    pipelineState.depthStencilState.enableDepthWrite = false;
    pipelineState.depthStencilState.depthComparisonFunc = eComparisonFunction::COMPARISON_FUNCTION_ALWAYS;

    pipelineState.inputLayout[0] = { 0, IMAGE_FORMAT_R32G32B32A32_FLOAT, 0, 0, 0, true, "POSITION" };
    pipelineState.inputLayout[1] = { 0, IMAGE_FORMAT_R32G32B32A32_FLOAT, 0, 0, 0, true, "COLOR" };

    pipelineState.renderPassLayout.attachements[0].stageBind = SHADER_STAGE_PIXEL;
    pipelineState.renderPassLayout.attachements[0].bindMode = RenderPassLayoutDesc::WRITE;
    pipelineState.renderPassLayout.attachements[0].targetState = RenderPassLayoutDesc::DONT_CARE;
    pipelineState.renderPassLayout.attachements[0].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

    pipelineState.resourceListLayout.resources[0] = { 0, SHADER_STAGE_VERTEX, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };

    renderLinePso = renderDevice->createPipelineState( pipelineState );
}

void LineRenderingModule::addLine( const nyaVec3f& from, const nyaVec3f& to, const float thickness, const nyaVec4f& color )
{
    if ( bufferIndex + 16 >= LINE_RENDERING_MAX_LINE_COUNT ) {
        return;
    }

    buffer[bufferIndex++] = ( from.x );
    buffer[bufferIndex++] = ( from.y );
    buffer[bufferIndex++] = ( from.z );
    buffer[bufferIndex++] = ( thickness );

    buffer[bufferIndex++] = ( color.x );
    buffer[bufferIndex++] = ( color.y );
    buffer[bufferIndex++] = ( color.z );
    buffer[bufferIndex++] = ( color.w );

    buffer[bufferIndex++] = ( to.x );
    buffer[bufferIndex++] = ( to.y );
    buffer[bufferIndex++] = ( to.z );
    buffer[bufferIndex++] = ( thickness );

    buffer[bufferIndex++] = ( color.x );
    buffer[bufferIndex++] = ( color.y );
    buffer[bufferIndex++] = ( color.z );
    buffer[bufferIndex++] = ( color.w );

    indiceCount += 2;
}
