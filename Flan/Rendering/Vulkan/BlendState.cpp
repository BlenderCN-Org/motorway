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
#include "BlendState.h"

#include "RenderContext.h"
#include "CommandList.h"

#include <Rendering/BlendOperations.h>
#include <Rendering/BlendSources.h>

using namespace flan::rendering;

static constexpr VkBlendFactor VK_BLEND_SOURCE[eBlendSource::BlendSource_COUNT] = {
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_FACTOR_ONE,

    VK_BLEND_FACTOR_SRC_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,

    VK_BLEND_FACTOR_SRC_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,

    VK_BLEND_FACTOR_DST_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,

    VK_BLEND_FACTOR_DST_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,

    VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,

    VK_BLEND_FACTOR_CONSTANT_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
};

static constexpr VkBlendOp VK_BLEND_OPERATION[eBlendOperation::BlendOperation_COUNT] = {
    VK_BLEND_OP_ADD,
    VK_BLEND_OP_SUBTRACT,
    VK_BLEND_OP_MIN,
    VK_BLEND_OP_MAX,
};

NativeBlendStateObject* flan::rendering::CreateBlendStateImpl( NativeRenderContext* nativeRenderContext, const BlendStateDesc& description )
{
    NativeBlendStateObject* blendState = new NativeBlendStateObject();

    VkPipelineColorBlendStateCreateInfo blendCreateInfos = {};
    blendCreateInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendCreateInfos.pNext = nullptr;
    blendCreateInfos.flags = 0;

    blendCreateInfos.attachmentCount = 8;
    blendCreateInfos.logicOpEnable = VK_FALSE;

    blendCreateInfos.blendConstants[0] = 1.0f;
    blendCreateInfos.blendConstants[1] = 1.0f;
    blendCreateInfos.blendConstants[2] = 1.0f;
    blendCreateInfos.blendConstants[3] = 1.0f;

    for ( int i = 0; i < 8; i++ ) {
        VkPipelineColorBlendAttachmentState blendAttachementState = {};

        blendAttachementState.blendEnable = ( description.enableBlend ) ? VK_TRUE : VK_FALSE;

        blendAttachementState.colorWriteMask = 0;

        if ( description.writeMask[0] )
            blendAttachementState.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
        else if ( description.writeMask[1] )
            blendAttachementState.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
        else if ( description.writeMask[2] )
            blendAttachementState.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
        else if ( description.writeMask[3] )
            blendAttachementState.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;

        blendAttachementState.srcColorBlendFactor = VK_BLEND_SOURCE[description.blendConfColor.source];
        blendAttachementState.dstColorBlendFactor = VK_BLEND_SOURCE[description.blendConfColor.dest];
        blendAttachementState.colorBlendOp = VK_BLEND_OPERATION[description.blendConfColor.operation];

        blendAttachementState.srcAlphaBlendFactor = VK_BLEND_SOURCE[description.blendConfAlpha.source];
        blendAttachementState.dstAlphaBlendFactor = VK_BLEND_SOURCE[description.blendConfAlpha.dest];
        blendAttachementState.alphaBlendOp = VK_BLEND_OPERATION[description.blendConfAlpha.operation];
    }

    blendState->nativeBlendState = blendCreateInfos;

    return blendState;
}

void flan::rendering::DestroyBlendStateImpl( NativeRenderContext* nativeRenderContext, NativeBlendStateObject* blendStateObject )
{
    // Pipeline state implicit destroy; nothing to see here...
}

void flan::rendering::BindBlendStateCmdImpl( NativeCommandList* nativeCmdList, NativeBlendStateObject* blendStateObject )
{
    // Pipeline state implicit bind; nothing to see here...
}
#endif
