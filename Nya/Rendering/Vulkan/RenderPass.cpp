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
#include <Rendering/RenderDevice.h>
#include "RenderDevice.h"

#include <Rendering/CommandList.h>
#include "CommandList.h"

#include "RenderPass.h"

#include "RenderTarget.h"
#include "Texture.h"
//#include "Buffer.h"

#include <Core/Allocators/PoolAllocator.h>

#include <vulkan/vulkan.h>

RenderPass* RenderDevice::createRenderPass( const RenderPassDesc& description )
{
    RenderPass* renderPass = nya::core::allocate<RenderPass>( renderContext->renderPassAllocator );

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0u;
    
    VkSubpassDescription subpassDesc = {};
    subpassDesc.flags = 0u;
    subpassDesc.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;

    for ( int i = 0; i < 24; i++ ) {
        const auto& attachment = description.attachements[i];

        switch ( attachment.bindMode ) {
        case RenderPassDesc::READ:
        {
            Texture* texture = nullptr;

            if ( attachment.targetState == RenderPassDesc::IS_TEXTURE )
                texture = attachment.texture;
           /* else if ( attachment.targetState == RenderPassDesc::IS_UAV_TEXTURE )
                texture = attachment.buffer->bufferTexture;*/
            else
                texture = attachment.renderTarget->texture;

            VkAttachmentDescription attachmentDesc = {};
            attachmentDesc.flags = 0u;
            attachmentDesc.format = texture->imageFormat;
            attachmentDesc.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

            attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkAttachmentReference attachmentReference = {};
            //attachmentReference.attachment = 
/*
            if ( attachment.stageBind & eShaderStage::SHADER_STAGE_PIXEL )
                renderPass->pixelStage.shaderResourceView[renderPass->pixelStage.srvCount++] = ( texture == nullptr ) ? nullptr : texture->shaderResourceView;
            if ( attachment.stageBind & eShaderStage::SHADER_STAGE_COMPUTE )
                renderPass->computeStage.shaderResourceView[renderPass->computeStage.srvCount++] = ( texture == nullptr ) ? nullptr : texture->shaderResourceView;
    */    } break;

        case RenderPassDesc::WRITE: {
            VkAttachmentDescription attachmentDesc = {};
            attachmentDesc.flags = 0u;
            attachmentDesc.format = attachment.renderTarget->texture->imageFormat;
            attachmentDesc.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

            attachmentDesc.loadOp = ( attachment.targetState == RenderPassDesc::DONT_CARE )
                ? VK_ATTACHMENT_LOAD_OP_DONT_CARE
                : VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            //renderPass->clearTarget[renderPass->rtvCount] = ( attachment.targetState != RenderPassDesc::DONT_CARE );
            //memcpy( renderPass->clearValue[renderPass->rtvCount], attachment.clearValue, sizeof( FLOAT ) * 4 );

            //if ( attachment.layerIndex != 0 )
            //    renderPass->renderTargetViews[renderPass->rtvCount++] = ( attachment.mipIndex != 0 )
            //    ? attachment.renderTarget->textureRenderTargetViewPerSliceAndMipLevel[attachment.layerIndex][attachment.mipIndex]
            //    : attachment.renderTarget->textureRenderTargetViewPerSlice[attachment.layerIndex];
            //else
            //    renderPass->renderTargetViews[renderPass->rtvCount++] = ( attachment.mipIndex != 0 )
            //    ? attachment.renderTarget->textureRenderTargetViewPerSliceAndMipLevel[0][attachment.mipIndex]
            //    : attachment.renderTarget->textureRenderTargetView;
           } break;

        case RenderPassDesc::WRITE_DEPTH: {
            VkAttachmentDescription attachmentDesc = {};
            attachmentDesc.flags = 0u;
            attachmentDesc.format = attachment.renderTarget->texture->imageFormat;
            attachmentDesc.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

            attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachmentDesc.stencilLoadOp = ( attachment.targetState == RenderPassDesc::DONT_CARE )
                ? VK_ATTACHMENT_LOAD_OP_DONT_CARE
                : VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference attachmentReference = {};
            attachmentReference.attachment = 0u;
            attachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            //renderPass->clearTarget[8] = ( attachment.targetState != RenderPassDesc::DONT_CARE );
            //memcpy( renderPass->clearValue[8], attachment.clearValue, sizeof( FLOAT ) * 4 );

            //renderPass->depthStencilView = attachment.renderTarget->textureDepthRenderTargetView;
        } break;

        default:
            break;
        }
    }

    vkCreateRenderPass( renderContext->device, &renderPassInfo, nullptr, &renderPass->renderPass );

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

    return renderPass;
}

void RenderDevice::destroyRenderPass( RenderPass* renderPass )
{
    vkDestroyRenderPass( renderContext->device, renderPass->renderPass, nullptr );
    nya::core::free( renderContext->renderPassAllocator, renderPass );
}

void CommandList::useRenderPass( RenderPass* renderPass )
{
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.renderPass = renderPass->renderPass;
    renderPassInfo.framebuffer = renderPass->framebuffer;
    renderPassInfo.clearValueCount = 24;
    renderPassInfo.pClearValues = renderPass->clearValues;

    vkCmdBeginRenderPass( CommandListObject->cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
}
#endif
