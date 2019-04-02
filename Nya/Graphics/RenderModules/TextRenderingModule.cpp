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

#include <Io/FontDescriptor.h>

#include <Graphics/RenderPipeline.h>
#include <Graphics/GraphicsAssetCache.h>
#include <Graphics/ShaderCache.h>

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

using namespace nya::rendering;

TextRenderingModule::TextRenderingModule()
    : fontAtlas( nullptr )
    , fontDescriptor( nullptr )
    , renderTextPso( nullptr )
    , indiceCount( 0 )
    , vertexBufferIndex( 0 )
    , glyphVertexBuffers{ nullptr, nullptr }
    , glyphIndiceBuffer( nullptr )
    , bufferOffset( 0 )
    , buffer{ 0.0f }
{

}

TextRenderingModule::~TextRenderingModule()
{
    fontAtlas = nullptr;
    fontDescriptor = nullptr;

    indiceCount = 0;
    vertexBufferIndex = 0;

    bufferOffset = 0;
}

void TextRenderingModule::destroy( RenderDevice* renderDevice )
{
    for ( int32_t i = 0; i < BUFFER_COUNT; i++ ) {
        renderDevice->destroyBuffer( glyphVertexBuffers[i] );
    }

    renderDevice->destroyBuffer( glyphIndiceBuffer );
    renderDevice->destroyPipelineState( renderTextPso );
}

MutableResHandle_t TextRenderingModule::renderText( RenderPipeline* renderPipeline, MutableResHandle_t output )
{
    struct PassData {
        MutableResHandle_t  output;

        ResHandle_t         viewportBuffer;
        ResHandle_t         bilinearSampler;
    };

    PassData& data = renderPipeline->addRenderPass<PassData>(
        "Text Rendering Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            renderPipelineBuilder.setUncullablePass();

            BufferDesc passBuffer;
            passBuffer.type = BufferDesc::CONSTANT_BUFFER;
            passBuffer.size = sizeof( nyaVec4u );

            passData.viewportBuffer = renderPipelineBuilder.allocateBuffer( passBuffer, eShaderStage::SHADER_STAGE_VERTEX );

            // Sampler State
            SamplerDesc bilinearSamplerDesc;
            bilinearSamplerDesc.addressU = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressV = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.addressW = eSamplerAddress::SAMPLER_ADDRESS_CLAMP_EDGE;
            bilinearSamplerDesc.filter = eSamplerFilter::SAMPLER_FILTER_BILINEAR;

            passData.bilinearSampler = renderPipelineBuilder.allocateSampler( bilinearSamplerDesc );

            // Passthrough rendertarget
            passData.output = renderPipelineBuilder.readRenderTarget( output );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, CommandList* cmdList ) {
            // Render Pass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );

            // Resource List
            Sampler* bilinearSampler = renderPipelineResources.getSampler( passData.bilinearSampler );
            
            Buffer* viewportBuffer = renderPipelineResources.getBuffer( passData.viewportBuffer );

            const Viewport* pipelineDimensions = renderPipelineResources.getMainViewport();
            nyaVec4u rtDimensions = {
                static_cast< uint32_t >( pipelineDimensions->Width ),
                static_cast< uint32_t >( pipelineDimensions->Height ),
                0u,
                0u
            };
            cmdList->updateBuffer( viewportBuffer, &rtDimensions, sizeof( nyaVec4u ) );

            ResourceList resourceList;
            resourceList.resource[0].buffer = viewportBuffer;
            resourceList.resource[1].sampler = bilinearSampler;
            resourceList.resource[2].texture = fontAtlas;

            RenderPass renderPass;
            renderPass.attachement[0] = { outputTarget, 0, 0 };

            // Update vertex buffer content
            cmdList->updateBuffer( glyphVertexBuffers[vertexBufferIndex], buffer, static_cast<size_t>( bufferOffset ) * sizeof( float ) );

            // Bind buffers
            cmdList->bindVertexBuffer( glyphVertexBuffers[vertexBufferIndex] );
            cmdList->bindIndiceBuffer( glyphIndiceBuffer );

            // Pipeline State
            cmdList->bindPipelineState( renderTextPso );
            cmdList->bindRenderPass( renderTextPso, renderPass );
            cmdList->bindResourceList( renderTextPso, resourceList );

            cmdList->drawIndexed( static_cast<uint32_t>( indiceCount ) );

            // Swap buffers
            vertexBufferIndex = ( vertexBufferIndex == 0 ) ? 1 : 0;

            // Reset buffers
            indiceCount = 0;
            bufferOffset = 0;
        } );

    return data.output;
}

void TextRenderingModule::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache )
{
    // Load Default Font
    fontDescriptor = graphicsAssetCache->getFont( NYA_STRING( "GameData/fonts/SegoeUI.fnt" ) );

    if ( fontDescriptor == nullptr ) {
        NYA_CERR << "Could not load default font descriptor!" << std::endl;
        return;
    }

    fontAtlas = graphicsAssetCache->getTexture( fontDescriptor->Name.c_str() );

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
    indiceBufferDescription.type = BufferDesc::INDICE_BUFFER;
    indiceBufferDescription.size = indexBufferSize;

    glyphIndiceBuffer = renderDevice->createBuffer( indiceBufferDescription, indexBufferData );

    // Create dynamic vertex buffer
    // 4 + 2 + 4
    constexpr std::size_t SINGLE_VERTEX_SIZE = sizeof( float ) * 10;

    // XYZ Position + RGBA Color + XY TexCoords (w/ 6 vertices per glyph)
    constexpr std::size_t VERTEX_BUFFER_SIZE = SINGLE_VERTEX_SIZE * TEXT_RENDERING_MAX_GLYPH_COUNT;

    BufferDesc bufferDescription;
    bufferDescription.type = BufferDesc::DYNAMIC_VERTEX_BUFFER;
    bufferDescription.size = VERTEX_BUFFER_SIZE;
    bufferDescription.stride = SINGLE_VERTEX_SIZE;

    for ( int32_t i = 0; i < BUFFER_COUNT; i++ ) {
        glyphVertexBuffers[i] = renderDevice->createBuffer( bufferDescription );
    }

    PipelineStateDesc pipelineState = {};
    pipelineState.vertexShader = shaderCache->getOrUploadStage( "UI/SDFTextRendering", eShaderStage::SHADER_STAGE_VERTEX );
    pipelineState.pixelShader = shaderCache->getOrUploadStage( "UI/SDFTextRendering", eShaderStage::SHADER_STAGE_PIXEL );

    pipelineState.primitiveTopology = ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    pipelineState.blendState.enableBlend = true;
    pipelineState.blendState.sampleMask = ~0u;

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

    pipelineState.inputLayout[0] = { 0, IMAGE_FORMAT_R32G32B32A32_FLOAT, 0, 0, 0, true, "POSITION" };
    pipelineState.inputLayout[1] = { 0, IMAGE_FORMAT_R32G32_FLOAT, 0, 0,  0, true, "TEXCOORD" };
    pipelineState.inputLayout[2] = { 0, IMAGE_FORMAT_R32G32B32A32_FLOAT, 0, 0, 0, true, "COLOR" };

    pipelineState.renderPassLayout.attachements[0].stageBind = SHADER_STAGE_PIXEL;
    pipelineState.renderPassLayout.attachements[0].bindMode = RenderPassLayoutDesc::WRITE;
    pipelineState.renderPassLayout.attachements[0].targetState = RenderPassLayoutDesc::DONT_CARE;
    pipelineState.renderPassLayout.attachements[0].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

    pipelineState.resourceListLayout.resources[0] =  { 0, SHADER_STAGE_VERTEX, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
    pipelineState.resourceListLayout.resources[1] =  { 0, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER };
    pipelineState.resourceListLayout.resources[2] =  { 0, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_TEXTURE };

    renderTextPso = renderDevice->createPipelineState( pipelineState );
}

void TextRenderingModule::addOutlinedText( const char* text, float size, float x, float y, const nyaVec4f& textColor, const float outlineThickness )
{
    const float baseX = x, localSize = size;
    float localX = x, localY = y;

    const auto localTextColor = textColor;

    int charIdx = 0;
    for ( const char* p = text; *p != '\0'; p++, charIdx++ ) {
        auto g = fontDescriptor->Glyphes[static_cast<size_t>( *p )];

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

        buffer[bufferOffset++] = ( gx );
        buffer[bufferOffset++] = ( gy );
        buffer[bufferOffset++] = ( 1.0f );
        buffer[bufferOffset++] = ( outlineThickness );
        buffer[bufferOffset++] = ( u1 );
        buffer[bufferOffset++] = ( v1 );
        buffer[bufferOffset++] = ( localTextColor.x );
        buffer[bufferOffset++] = ( localTextColor.y );
        buffer[bufferOffset++] = ( localTextColor.z );
        buffer[bufferOffset++] = ( localTextColor.w );

        buffer[bufferOffset++] = ( gx );
        buffer[bufferOffset++] = ( gy - gh );
        buffer[bufferOffset++] = ( 1.0f );
        buffer[bufferOffset++] = ( outlineThickness );
        buffer[bufferOffset++] = ( u1 );
        buffer[bufferOffset++] = ( v2 );
        buffer[bufferOffset++] = ( localTextColor.x );
        buffer[bufferOffset++] = ( localTextColor.y );
        buffer[bufferOffset++] = ( localTextColor.z );
        buffer[bufferOffset++] = ( localTextColor.w );

        buffer[bufferOffset++] = ( gx + gw );
        buffer[bufferOffset++] = ( gy - gh );
        buffer[bufferOffset++] = ( 1.0f );
        buffer[bufferOffset++] = ( outlineThickness );
        buffer[bufferOffset++] = ( u2 );
        buffer[bufferOffset++] = ( v2 );
        buffer[bufferOffset++] = ( localTextColor.x );
        buffer[bufferOffset++] = ( localTextColor.y );
        buffer[bufferOffset++] = ( localTextColor.z );
        buffer[bufferOffset++] = ( localTextColor.w );

        buffer[bufferOffset++] = ( gx + gw );
        buffer[bufferOffset++] = ( gy );
        buffer[bufferOffset++] = ( 1.0f );
        buffer[bufferOffset++] = ( outlineThickness );
        buffer[bufferOffset++] = ( u2 );
        buffer[bufferOffset++] = ( v1 );
        buffer[bufferOffset++] = ( localTextColor.x );
        buffer[bufferOffset++] = ( localTextColor.y );
        buffer[bufferOffset++] = ( localTextColor.z );
        buffer[bufferOffset++] = ( localTextColor.w );

        indiceCount += 6;
    }
}
