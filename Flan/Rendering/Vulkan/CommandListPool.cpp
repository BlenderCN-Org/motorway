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
#include "CommandListPool.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include "CommandList.h"
#include "RenderContext.h"

NativeCommandListPool* flan::rendering::CreateCommandListPoolImpl( RenderDevice* renderDevice, const int poolCapacity, const CommandListPool::eQueueType poolDeviceQueue )
{
    auto nativeRenderContext = renderDevice->getNativeRenderContext();

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = 0;

    switch ( poolDeviceQueue ) {
    case CommandListPool::GRAPHICS_QUEUE:
        poolInfo.queueFamilyIndex = nativeRenderContext->graphicsQueueIndex;
        break;
    case CommandListPool::COMPUTE_QUEUE:
        poolInfo.queueFamilyIndex = nativeRenderContext->computeQueueIndex;
        break;
    default:
        break;
    }

    VkCommandPool commandPool;
    VkResult operationResult = vkCreateCommandPool( nativeRenderContext->device, &poolInfo, nullptr, &commandPool );
    if ( operationResult != VK_SUCCESS ) {
        FLAN_CERR << "Failed to create CommandPool! (error code: " << operationResult << ")" << std::endl;
        return nullptr;
    }

    NativeCommandListPool* cmdListPool = new NativeCommandListPool();
    cmdListPool->commandPool = commandPool;

    return cmdListPool;
}

void flan::rendering::DestroyCommandListPoolImpl( RenderDevice* renderDevice, NativeCommandListPool* cmdListPool )
{
    auto nativeRenderContext = renderDevice->getNativeRenderContext();

    vkDestroyCommandPool( nativeRenderContext->device, cmdListPool->commandPool, nullptr );
}

CommandList* flan::rendering::AllocateCommandListPoolImpl( RenderDevice* renderDevice, NativeCommandListPool* cmdListPool )
{
    auto nativeRenderContext = renderDevice->getNativeRenderContext();

    VkCommandBufferAllocateInfo cmdAllocInfos;
    cmdAllocInfos.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfos.pNext = nullptr;
    cmdAllocInfos.commandBufferCount = 1;
    cmdAllocInfos.commandPool = cmdListPool->commandPool;
    cmdAllocInfos.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers( nativeRenderContext->device, &cmdAllocInfos, &cmdBuffer );

    auto nativeCmdList = new NativeCommandList();
    nativeCmdList->cmdBuffer = cmdBuffer;

    auto cmdList = new CommandList();
    cmdList->setNativeCommandList( nativeCmdList );

    return cmdList;
}
#endif
