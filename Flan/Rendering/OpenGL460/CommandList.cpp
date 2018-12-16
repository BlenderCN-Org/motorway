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

#if FLAN_GL460
#include "CommandList.h"
#include "RenderContext.h"

#include "PipelineState.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "Buffer.h"

static constexpr GLenum GL_PRIMITIVE_TOPOLOGY[flan::rendering::ePrimitiveTopology::PrimitiveTopology_COUNT] = {
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_LINE
};

NativeCommandList* flan::rendering::CreateCommandListImpl( NativeRenderContext* renderContext )
{
    NativeCommandList* cmdList = new NativeCommandList();

    // Generate a bunch of framebuffers to simulate render targets correctly
    glCreateFramebuffers( NativeCommandList::FBO_INTERNAL_COUNT, cmdList->FrameBuffers );
    cmdList->FrameBufferAvailableIndex = 0;

    for ( int32_t i = 0; i < NativeCommandList::FBO_INTERNAL_COUNT; i++ )
        for ( int32_t attachement = 0; attachement < 9; attachement++ )
            cmdList->FrameBuffersAttachementsUsed[i][attachement] = 0;

    // Generate dummy vertex array
    // NOTE Seems we could create a VAO without any VBO
    // But since OpenGL implementations are soooo consistent between platforms and OS (hint: that's sarcasm)
    // keep this as verbose as possible
    GLuint vao, vbo;
    glCreateVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glCreateBuffers( 1, &vbo );
    glNamedBufferData( vbo, 0, nullptr, GL_STATIC_DRAW );
    glBindVertexBuffer( 0, vbo, 0, 0 );
    glBindVertexArray( 0 );

    cmdList->PrimitiveTopology = GL_TRIANGLES;

    cmdList->DummyVAO = vao;
    cmdList->DummyVBO = vbo;

    return cmdList;
}

void flan::rendering::DestroyCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{
    glDeleteFramebuffers( NativeCommandList::FBO_INTERNAL_COUNT, cmdList->FrameBuffers );
}

void flan::rendering::BeginCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{

}

void flan::rendering::EndCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{

}

void flan::rendering::PlayBackCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{
    // TODO Emulate command list in opengl (accumulate cmds and replay them whenever the function is called)
}

void flan::rendering::SetDepthClearValue( NativeCommandList* cmdList, const float depthClearValue )
{
    glClearDepth( static_cast<GLclampd>( depthClearValue ) );

    // HACK Disabling depth write also disable depth clear (in OpenGL, not the case in DirectX)
    // TODO Could be nice to create an unbind function and forces depth write by default (so that we dont have a
    // state change everytime the depth clear value is changed...)
    glDepthMask( GL_TRUE );
}

void flan::rendering::ClearRenderTargetImpl( NativeCommandList* cmdList )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}

void flan::rendering::ClearColorRenderTargetImpl( NativeCommandList* cmdList )
{
    glClear( GL_COLOR_BUFFER_BIT );
}

void flan::rendering::ClearDepthRenderTargetImpl( NativeCommandList* cmdList )
{
    glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}

void flan::rendering::DrawImpl( NativeCommandList* cmdList, const unsigned int vertexCount, const unsigned int vertexOffset )
{
    glDrawArrays( cmdList->PrimitiveTopology, static_cast<GLint>( vertexOffset ), static_cast<GLsizei>( vertexCount ) );
}

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

void flan::rendering::DrawIndexedImpl( NativeCommandList* cmdList, const uint32_t indiceCount, const uint32_t indiceOffset, const std::size_t indiceType, const uint32_t vertexOffset )
{
    glDrawElementsBaseVertex( cmdList->PrimitiveTopology, static_cast<GLsizei>( indiceCount ), GetGLIndiceType( indiceType ), static_cast<const char*>( nullptr ) + indiceOffset * indiceType, static_cast<GLint>( vertexOffset ) );
}

void flan::rendering::DrawInstancedIndexedImpl( NativeCommandList* cmdList, const unsigned int indiceCount, const unsigned int instanceCount, const unsigned int indexOffset, const unsigned int vertexOffset, const unsigned int instanceOffset )
{

}

void flan::rendering::DrawInstancedIndirectImpl( NativeCommandList* cmdList, const NativeBufferObject* drawArgsBuffer, const unsigned int bufferDataOffset )
{

}

void flan::rendering::DispatchComputeImpl( NativeCommandList* cmdList, const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ )
{
    glDispatchCompute( threadCountX, threadCountY, threadCountZ );
    glMemoryBarrier( GL_ALL_BARRIER_BITS );
}

void flan::rendering::GetViewportImpl( NativeCommandList* cmdList, Viewport& viewport )
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

void flan::rendering::SetViewportImpl( NativeCommandList* cmdList, const Viewport& viewport )
{
    glViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    glDepthRange( static_cast<GLclampd>( viewport.MinDepth ), static_cast<GLclampd>( viewport.MaxDepth ) );
}

void flan::rendering::BindBackbufferImpl( NativeCommandList* cmdList )
{
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );
    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
}

void flan::rendering::BindRenderTargetImpl( NativeCommandList* cmdList, RenderTarget** renderTarget, RenderTarget* depthRenderTarget, const uint32_t renderTargetCount, const uint32_t mipLevel )
{
    // Inc fbo index (do
    cmdList->FrameBufferAvailableIndex++;
    if ( cmdList->FrameBufferAvailableIndex == NativeCommandList::FBO_INTERNAL_COUNT ) {
        cmdList->FrameBufferAvailableIndex = 0;
    }

    const auto framebufferIndex = cmdList->FrameBufferAvailableIndex;
    GLuint framebufferHandle = cmdList->FrameBuffers[framebufferIndex];

    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, framebufferHandle );

    // Bind attachements
    for ( uint32_t targetId = 0; targetId < renderTargetCount; targetId++ ) {
        auto& currentAttachementHandle = cmdList->FrameBuffersAttachementsUsed[framebufferIndex][targetId];
        auto textureObject = renderTarget[targetId]->getNativeTextureObject();

        // If we reuse the same framebuffer, don't bother reattaching
        if ( currentAttachementHandle != textureObject->textureHandle ) {
            glNamedFramebufferTexture( framebufferHandle, static_cast< GLenum >( GL_COLOR_ATTACHMENT0 + targetId ), textureObject->textureHandle, static_cast<GLint>( mipLevel ) );
            currentAttachementHandle = textureObject->textureHandle;
        }
    }

    // Unbind attachements left
    for ( uint32_t targetId = renderTargetCount; targetId < 8; targetId++ ) {
        auto& currentAttachementHandle = cmdList->FrameBuffersAttachementsUsed[framebufferIndex][targetId];
        if ( currentAttachementHandle != 0 ) {
            glNamedFramebufferTexture( framebufferHandle, static_cast< GLenum >( GL_COLOR_ATTACHMENT0 + targetId ), 0, 0 );
            cmdList->FrameBuffersAttachementsUsed[framebufferIndex][targetId] = 0;
        }
    }

    // Bind depth buffer (if available)
    if ( depthRenderTarget != nullptr ) {
        auto depthTextureObject = depthRenderTarget->getNativeTextureObject();
        glNamedFramebufferTexture( framebufferHandle, GL_DEPTH_ATTACHMENT, depthTextureObject->textureHandle, static_cast<GLint>( mipLevel ) );

        cmdList->FrameBuffersAttachementsUsed[framebufferIndex][8] = depthTextureObject->textureHandle;
    } else if ( cmdList->FrameBuffersAttachementsUsed[framebufferIndex][8] != 0 ) {
        glNamedFramebufferTexture( framebufferHandle, GL_DEPTH_ATTACHMENT, 0, 0 );

        cmdList->FrameBuffersAttachementsUsed[framebufferIndex][8] = 0;
    }

    const bool isDepthOnly = ( ( depthRenderTarget != nullptr ) && renderTargetCount == 0 );
    if ( isDepthOnly ) {
        glDrawBuffer( GL_NONE );
    } else {
        //glDrawBuffer( GL_BACK );

        // MRT: max 8 targets per fbo (+1 depth attachement)
        constexpr GLenum BUFFERS[8] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
            GL_COLOR_ATTACHMENT5,
            GL_COLOR_ATTACHMENT6,
            GL_COLOR_ATTACHMENT7,
        };

        glDrawBuffers( static_cast<GLsizei>( renderTargetCount ), BUFFERS );
    }

    GLenum framebufferStatus = GL_FRAMEBUFFER_COMPLETE;
    if ( ( framebufferStatus = glCheckNamedFramebufferStatus( framebufferHandle, GL_FRAMEBUFFER ) ) != GL_FRAMEBUFFER_COMPLETE ) {
        FLAN_CERR << "Incomplete framebuffer! (error code: " << framebufferStatus << ")"  << std::endl;
       // FLAN_CERR << renderTargetCount << " color target(s) " << ( ( depthRenderTarget != nullptr ) ? 1 : 0 ) << " depth target" << std::endl;

        BindBackbufferImpl( cmdList );
        return;
    }
}

void flan::rendering::BindRenderTargetLayeredImpl( NativeCommandList* cmdList, RenderTarget** renderTarget, RenderTarget* depthRenderTarget, const uint32_t renderTargetCount, const uint32_t mipLevel, const uint32_t layerIndex )
{
    const auto framebufferIndex = cmdList->FrameBufferAvailableIndex;
    GLuint framebufferHandle = cmdList->FrameBuffers[framebufferIndex];

    glBindFramebuffer( GL_DRAW_FRAMEBUFFER, framebufferHandle );

    // Bind attachements
    for ( uint32_t targetId = 0; targetId < renderTargetCount; targetId++ ) {
        auto& currentAttachementHandle = cmdList->FrameBuffersAttachementsUsed[framebufferIndex][targetId];
        auto textureObject = renderTarget[targetId]->getNativeTextureObject();

        // If we reuse the same framebuffer, don't bother reattaching
        //if ( currentAttachementHandle != textureObject->textureHandle ) {
            glNamedFramebufferTextureLayer( framebufferHandle, static_cast< GLenum >( GL_COLOR_ATTACHMENT0 + targetId ), textureObject->textureHandle, static_cast<GLint>( mipLevel ), static_cast<GLint>( layerIndex ) );
            currentAttachementHandle = textureObject->textureHandle;
        //}
    }

    // Unbind attachements left
    for ( uint32_t targetId = renderTargetCount; targetId < 8; targetId++ ) {
        auto& currentAttachementHandle = cmdList->FrameBuffersAttachementsUsed[framebufferIndex][targetId];
        if ( currentAttachementHandle != 0 ) {
            glNamedFramebufferTextureLayer( framebufferHandle, static_cast< GLenum >( GL_COLOR_ATTACHMENT0 + targetId ), 0, 0, 0);
            cmdList->FrameBuffersAttachementsUsed[framebufferIndex][targetId] = 0;
        }
    }

    // Bind depth buffer (if available)
    if ( depthRenderTarget != nullptr ) {
        auto depthTextureObject = depthRenderTarget->getNativeTextureObject();
        glNamedFramebufferTextureLayer( framebufferHandle, GL_DEPTH_ATTACHMENT, depthTextureObject->textureHandle, static_cast<GLint>( mipLevel ), static_cast<GLint>( layerIndex ) );

        cmdList->FrameBuffersAttachementsUsed[framebufferIndex][8] = depthTextureObject->textureHandle;
    } else if ( cmdList->FrameBuffersAttachementsUsed[framebufferIndex][8] != 0 ) {
        glNamedFramebufferTextureLayer( framebufferHandle, GL_DEPTH_ATTACHMENT, 0, 0, 0 );

        cmdList->FrameBuffersAttachementsUsed[framebufferIndex][8] = 0;
    }

    const bool isDepthOnly = ( ( depthRenderTarget != nullptr ) && renderTargetCount == 0 );
    if ( isDepthOnly ) {
        glDrawBuffer( GL_NONE );
    } else {
        //glDrawBuffer( GL_BACK );

        // MRT: max 8 targets per fbo (+1 depth attachement)
        constexpr GLenum BUFFERS[8] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
            GL_COLOR_ATTACHMENT5,
            GL_COLOR_ATTACHMENT6,
            GL_COLOR_ATTACHMENT7,
        };

        glDrawBuffers( static_cast<GLsizei>( renderTargetCount ), BUFFERS );
    }

    cmdList->FrameBufferAvailableIndex++;
    if ( cmdList->FrameBufferAvailableIndex == NativeCommandList::FBO_INTERNAL_COUNT ) {
        cmdList->FrameBufferAvailableIndex = 0;
    }

    GLenum framebufferStatus = GL_FRAMEBUFFER_COMPLETE;
    if ( ( framebufferStatus = glCheckNamedFramebufferStatus( framebufferHandle, GL_FRAMEBUFFER ) ) != GL_FRAMEBUFFER_COMPLETE ) {

        if ( GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT == framebufferStatus )
            FLAN_CERR << "Incomplete framebuffer! (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)"  << std::endl;
        else
            FLAN_CERR << "Incomplete framebuffer! (error code: " << framebufferStatus << ")"  << std::endl;
       // FLAN_CERR << renderTargetCount << " color target(s) " << ( ( depthRenderTarget != nullptr ) ? 1 : 0 ) << " depth target" << std::endl;

        BindBackbufferImpl( cmdList );

        return;
    }
}

void flan::rendering::BindPipelineStateImpl( NativeCommandList* cmdList, PipelineState* pipelineState )
{
    const auto nativePipelineState = pipelineState->getNativePipelineStateObject();
    const auto psoDesc = pipelineState->getDescription();

    glUseProgram( nativePipelineState->renderProgram );
    cmdList->PrimitiveTopology = GL_PRIMITIVE_TOPOLOGY[psoDesc->primitiveTopology];
}

void flan::rendering::UnbindVertexArrayImpl( NativeCommandList* cmdList )
{
    glBindVertexArray( cmdList->DummyVAO );
}

void flan::rendering::ResolveSubresourceImpl( NativeCommandList* cmdList, NativeTextureObject* resourceToResolve, NativeTextureObject* resolvedResource )
{

}
#endif
