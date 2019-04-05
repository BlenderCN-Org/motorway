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

#include <Maths/Helpers.h>

#include <vulkan/vulkan.h>

void CommandList::bindRenderPass( PipelineState* pipelineState, const RenderPass& renderPass )
{
    if ( CommandListObject->isRenderPassInProgress ) {
        vkCmdEndRenderPass( CommandListObject->cmdBuffer );
        CommandListObject->isRenderPassInProgress = false;
    }

    if ( pipelineState->framebuffer != VK_NULL_HANDLE ) {
        vkDestroyFramebuffer( CommandListObject->device, pipelineState->framebuffer, nullptr );
    }

    VkImageView imageViews[24];
    uint32_t attachmentCount = 0u;
    for ( ; attachmentCount < pipelineState->attachmentCount; attachmentCount++ ) {
        const auto& attachment = renderPass.attachement[attachmentCount];

        imageViews[attachmentCount] = attachment.renderTarget->textureRenderTargetViewPerSliceAndMipLevel[attachment.faceIndex][attachment.mipLevel];
    }

    VkFramebufferCreateInfo framebufferCreateInfos = {};
    framebufferCreateInfos.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfos.pNext = nullptr;
    framebufferCreateInfos.flags = 0u;
    framebufferCreateInfos.renderPass = pipelineState->renderPass;
    framebufferCreateInfos.attachmentCount = attachmentCount;
    framebufferCreateInfos.pAttachments = imageViews;
    framebufferCreateInfos.width = static_cast<uint32_t>( CommandListObject->currentViewport.Width );
    framebufferCreateInfos.height = static_cast<uint32_t>( CommandListObject->currentViewport.Height );
    framebufferCreateInfos.layers = 1u;

    vkCreateFramebuffer( CommandListObject->device, &framebufferCreateInfos, nullptr, &pipelineState->framebuffer );

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.renderPass = pipelineState->renderPass;
    renderPassInfo.framebuffer = pipelineState->framebuffer;
    renderPassInfo.clearValueCount = 24;
    renderPassInfo.pClearValues = pipelineState->clearValues;

    vkCmdBeginRenderPass( CommandListObject->cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

    CommandListObject->isRenderPassInProgress = true;
}
#endif
