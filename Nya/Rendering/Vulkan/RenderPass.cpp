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

#if NYA_VULKAN
#include <Rendering/CommandList.h>
#include "CommandList.h"

#include "RenderTarget.h"
#include "Texture.h"
#include "PipelineState.h"

#include <vulkan/vulkan.h>

void CommandList::bindRenderPass( const PipelineState* pipelineState, const RenderPass& resourceList )
{
    /*VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.renderPass = renderPass->renderPass;
    renderPassInfo.framebuffer = renderPass->framebuffer;
    renderPassInfo.clearValueCount = 24;
    renderPassInfo.pClearValues = renderPass->clearValues;

    vkCmdBeginRenderPass( CommandListObject->cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

    VkFramebufferCreateInfo framebufferCreateInfos = {};
    framebufferCreateInfos.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfos.pNext = nullptr;
    framebufferCreateInfos.flags = 0u;
    framebufferCreateInfos.renderPass = renderPass->renderPass;
    framebufferCreateInfos.attachmentCount = renderPass->attachmentCount;
    framebufferCreateInfos.pAttachments = renderPass->imageViews;
    framebufferCreateInfos.width = renderPass->width;
    framebufferCreateInfos.height = renderPass->height;
    framebufferCreateInfos.layers = 1u;

    vkCreateFramebuffer( renderContext->device, &framebufferCreateInfos, nullptr, &renderPass->framebuffer );
    */
}
#endif
