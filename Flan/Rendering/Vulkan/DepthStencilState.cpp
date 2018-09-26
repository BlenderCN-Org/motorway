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
#include "DepthStencilState.h"

#include <Rendering/StencilOperation.h>

#include "RenderContext.h"
#include "CommandList.h"
#include "ComparisonFunctions.h"

using namespace flan::rendering;

static constexpr VkStencilOp VK_STENCIL_OPERATION[eStencilOperation::StencilOperation_COUNT] =
{
    VK_STENCIL_OP_KEEP,
    VK_STENCIL_OP_ZERO,
    VK_STENCIL_OP_REPLACE,
    VK_STENCIL_OP_INCREMENT_AND_WRAP,
    VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    VK_STENCIL_OP_DECREMENT_AND_WRAP,
    VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    VK_STENCIL_OP_INVERT
};

NativeDepthStencilStateObject* flan::rendering::CreateDepthStencilStateImpl( NativeRenderContext* nativeRenderContext, const DepthStencilStateDesc& description )
{
    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfos;
    depthStencilStateInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfos.pNext = nullptr;
    depthStencilStateInfos.flags = 0;
    depthStencilStateInfos.depthTestEnable = static_cast< VkBool32 >( description.enableDepthTest );
    depthStencilStateInfos.depthWriteEnable = static_cast< VkBool32 >( description.enableDepthWrite );
    depthStencilStateInfos.depthCompareOp = VK_COMPARISON_FUNCTION[description.depthComparisonFunc];
    depthStencilStateInfos.depthBoundsTestEnable = description.enableDepthBoundsTest;
    depthStencilStateInfos.stencilTestEnable = description.enableStencilTest;

    VkStencilOpState frontStencilState;
    frontStencilState.failOp = VK_STENCIL_OPERATION[description.front.failOp];
    frontStencilState.passOp = VK_STENCIL_OPERATION[description.front.passOp];
    frontStencilState.depthFailOp = VK_STENCIL_OPERATION[description.front.zFailOp];
    frontStencilState.compareOp = VK_COMPARISON_FUNCTION[description.front.comparisonFunc];
    frontStencilState.compareMask = description.stencilReadMask;
    frontStencilState.writeMask = description.stencilWriteMask;
    frontStencilState.reference = description.stencilRefValue;
    depthStencilStateInfos.front = frontStencilState;

    VkStencilOpState backStencilState;
    backStencilState.failOp = VK_STENCIL_OPERATION[description.back.failOp];
    backStencilState.passOp = VK_STENCIL_OPERATION[description.back.passOp];
    backStencilState.depthFailOp = VK_STENCIL_OPERATION[description.back.zFailOp];
    backStencilState.compareOp = VK_COMPARISON_FUNCTION[description.back.comparisonFunc];
    backStencilState.compareMask = description.stencilReadMask;
    backStencilState.writeMask = description.stencilWriteMask;
    backStencilState.reference = description.stencilRefValue;
    depthStencilStateInfos.back = backStencilState;

    depthStencilStateInfos.minDepthBounds = description.depthBoundsMin;
    depthStencilStateInfos.maxDepthBounds = description.depthBoundsMax;

    NativeDepthStencilStateObject* depthStencilState = new NativeDepthStencilStateObject();
    depthStencilState->nativeDepthStencilState = depthStencilStateInfos;
    return depthStencilState;
}

void flan::rendering::DestroyDepthStencilStateImpl( NativeRenderContext* nativeRenderContext, NativeDepthStencilStateObject* depthStencilStateObject )
{
    // Pipeline state implicit destroy; nothing to see here...
}

void flan::rendering::BindDepthStencilStateCmdImpl( NativeCommandList* nativeCommandList, NativeDepthStencilStateObject* depthStencilStateObject )
{
    // Pipeline state implicit destroy; nothing to see here...
}
#endif
