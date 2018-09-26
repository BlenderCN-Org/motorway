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

#if FLAN_VULKAN
#include "CommandList.h"
#include "RenderContext.h"

#include "PipelineState.h"
#include "RenderTarget.h"

void flan::rendering::BeginCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{
    VkCommandBufferInheritanceInfo vkCmdBufferInheritanceInfos = {};
    vkCmdBufferInheritanceInfos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    vkCmdBufferInheritanceInfos.pNext = nullptr;
    
    VkCommandBufferBeginInfo vkCmdBufferInfos = {};
    vkCmdBufferInfos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCmdBufferInfos.pNext = nullptr;
    vkCmdBufferInfos.flags = 0;
    vkCmdBufferInfos.pInheritanceInfo = &vkCmdBufferInheritanceInfos;

    vkBeginCommandBuffer( cmdList->cmdBuffer, &vkCmdBufferInfos );
}

void flan::rendering::EndCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{
    vkEndCommandBuffer( cmdList->cmdBuffer );
}

void flan::rendering::PlayBackCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{

}

void flan::rendering::SetDepthClearValue( NativeCommandList* cmdList, const float depthClearValue )
{

}

void flan::rendering::ClearRenderTargetImpl( NativeCommandList* cmdList )
{

}

void flan::rendering::ClearColorRenderTargetImpl( NativeCommandList* cmdList )
{

}

void flan::rendering::ClearDepthRenderTargetImpl( NativeCommandList* cmdList )
{

}

void flan::rendering::DrawImpl( NativeCommandList* cmdList, const unsigned int vertexCount, const unsigned int vertexOffset )
{
    vkCmdDraw( cmdList->cmdBuffer, vertexCount, 1, vertexOffset, 0 );
}

void flan::rendering::DrawIndexedImpl( NativeCommandList* cmdList, const uint32_t indiceCount, const uint32_t indiceOffset, const std::size_t indiceType, const uint32_t vertexOffset )
{
    vkCmdDrawIndexed( cmdList->cmdBuffer, indiceCount, 1, indiceOffset, vertexOffset, 0 );
}

void flan::rendering::DrawInstancedIndexedImpl( NativeCommandList* cmdList, const unsigned int indiceCount, const unsigned int instanceCount, const unsigned int indexOffset, const unsigned int vertexOffset, const unsigned int instanceOffset )
{
    vkCmdDrawIndexed( cmdList->cmdBuffer, indiceCount, instanceCount, indexOffset, vertexOffset, instanceOffset );
}

void flan::rendering::DispatchComputeImpl( NativeCommandList* cmdList, const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ )
{
    vkCmdDispatch( cmdList->cmdBuffer, threadCountX, threadCountY, threadCountZ );
}

void flan::rendering::GetViewportImpl( NativeCommandList* cmdList, Viewport& viewport )
{
    viewport = cmdList->currentViewport;
}

void flan::rendering::SetViewportImpl( NativeCommandList* cmdList, const Viewport& viewport )
{
    cmdList->currentViewport = viewport;

    VkViewport vkViewport;
    vkViewport.x = static_cast<float>( viewport.X );
    vkViewport.y = static_cast<float>( viewport.Y );
    vkViewport.width = static_cast<float>( viewport.Width );
    vkViewport.height = static_cast<float>( viewport.Height );
    vkViewport.minDepth = viewport.MinDepth;
    vkViewport.maxDepth = viewport.MaxDepth;

    vkCmdSetViewport( cmdList->cmdBuffer, 0, 1, &vkViewport );
}

void flan::rendering::BindBackbufferImpl( NativeCommandList* cmdList )
{

}

void flan::rendering::BindRenderTargetImpl( NativeCommandList* cmdList, RenderTarget** renderTarget, RenderTarget* depthRenderTarget, const uint32_t renderTargetCount, const uint32_t mipLevel )
{

}

void flan::rendering::BindRenderTargetLayeredImpl( NativeCommandList* cmdList, RenderTarget** renderTarget, RenderTarget* depthRenderTarget, const uint32_t renderTargetCount, const uint32_t mipLevel, const uint32_t layerIndex )
{

}

void flan::rendering::UnbindVertexArrayImpl( NativeCommandList* cmdList )
{

}

void flan::rendering::BindPipelineStateImpl( NativeCommandList* cmdList, PipelineState* pipelineState )
{

}
#endif
