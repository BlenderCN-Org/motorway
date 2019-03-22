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
#include <Rendering/CommandList.h>

#include "RenderDevice.h"
#include "CommandList.h"
#include "Shader.h"
#include "ComparisonFunctions.h"

#include <vulkan/vulkan.h>

using namespace nya::rendering;

static constexpr VkPrimitiveTopology VK_PRIMITIVE_TOPOLOGY[ePrimitiveTopology::PRIMITIVE_TOPOLOGY_COUNT] =
{
    VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
};

static constexpr VkBlendFactor VK_BLEND_SOURCE[eBlendSource::BLEND_SOURCE_COUNT] =
{
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

static constexpr VkBlendOp VK_BLEND_OPERATION[eBlendOperation::BLEND_OPERATION_COUNT] =
{
    VK_BLEND_OP_ADD,
    VK_BLEND_OP_SUBTRACT,
    VK_BLEND_OP_MIN,
    VK_BLEND_OP_MAX,
};

static constexpr VkStencilOp VK_STENCIL_OPERATION[eStencilOperation::STENCIL_OPERATION_COUNT] =
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

static constexpr VkPolygonMode VK_FM[eFillMode::FILL_MODE_COUNT] =
{
    VkPolygonMode::VK_POLYGON_MODE_FILL,
    VkPolygonMode::VK_POLYGON_MODE_LINE,
};

static constexpr VkCullModeFlags VK_CM[eCullMode::CULL_MODE_COUNT] =
{
    VK_CULL_MODE_NONE,
    VK_CULL_MODE_FRONT_BIT,
    VK_CULL_MODE_BACK_BIT,
    VK_CULL_MODE_FRONT_AND_BACK,
};

struct PipelineState
{
    VkPipeline  nativePipelineObject;
};

PipelineState* RenderDevice::createPipelineState( const PipelineStateDesc& description )
{
    PipelineState* pipelineState = nya::core::allocate<PipelineState>( memoryAllocator );

    return pipelineState;
}

void RenderDevice::destroyPipelineState( PipelineState* pipelineState )
{
    nya::core::free( memoryAllocator, pipelineState );
}

void CommandList::bindPipelineState( PipelineState* pipelineState )
{

}
#endif
