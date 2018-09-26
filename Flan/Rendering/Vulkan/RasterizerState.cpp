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
#include "RasterizerState.h"
#include "RenderContext.h"
#include "CommandList.h"

#include <Rendering/FillModes.h>
#include <Rendering/CullModes.h>

using namespace flan::rendering;

static constexpr VkPolygonMode VK_FM[FillMode_COUNT] = {
    VkPolygonMode::VK_POLYGON_MODE_FILL,
    VkPolygonMode::VK_POLYGON_MODE_LINE,
};

static constexpr VkCullModeFlags VK_CM[CullMode_COUNT] = {
    VK_CULL_MODE_NONE,
    VK_CULL_MODE_FRONT_BIT,
    VK_CULL_MODE_BACK_BIT,
    VK_CULL_MODE_FRONT_AND_BACK,
};

NativeRasterizerStateObject* flan::rendering::CreateRasterizerStateImpl( NativeRenderContext* nativeRenderContext, const RasterizerStateDesc& description )
{
    VkPipelineRasterizationStateCreateInfo rasterizerStateInfos;
    rasterizerStateInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateInfos.pNext = VK_NULL_HANDLE;
    rasterizerStateInfos.flags = 0;
    rasterizerStateInfos.depthClampEnable = static_cast< VkBool32 >( description.depthBiasClamp != 0.0f );
    rasterizerStateInfos.rasterizerDiscardEnable = VK_FALSE;
    rasterizerStateInfos.polygonMode = VK_FM[description.fillMode];
    rasterizerStateInfos.cullMode = VK_CM[description.cullMode];
    rasterizerStateInfos.frontFace = ( description.useTriangleCCW ) ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizerStateInfos.depthBiasEnable = static_cast< VkBool32 >( description.depthBias != 0.0f );
    rasterizerStateInfos.depthBiasConstantFactor = description.depthBias;
    rasterizerStateInfos.depthBiasClamp = description.depthBiasClamp;
    rasterizerStateInfos.depthBiasSlopeFactor = description.slopeScale;
    rasterizerStateInfos.lineWidth = 1.0f;

    NativeRasterizerStateObject* rasterizerState = new NativeRasterizerStateObject();
    rasterizerState->nativeRasterizerState = rasterizerStateInfos;

    return rasterizerState;
}

void flan::rendering::DestroyRasterizerStateImpl( NativeRenderContext* nativeRenderContext, NativeRasterizerStateObject* rasterizerStateObject )
{
    // Implicit Pipeline state; nothing to destroy...
}

void flan::rendering::BindRasterizerStateCmdImpl( NativeCommandList* nativeCmdList, NativeRasterizerStateObject* rasterizerStateObject )
{
    // Implicit Pipeline state; nothing to bind...
}
#endif
