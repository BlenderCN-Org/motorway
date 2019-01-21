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
#pragma once

#if NYA_VULKAN
#include <Rendering/RenderDevice.h>
#include <vulkan/vulkan.h>

using namespace nya::rendering;

static constexpr VkCompareOp VK_COMPARISON_FUNCTION[eComparisonFunction::COMPARISON_FUNCTION_COUNT] =
{
    VkCompareOp::VK_COMPARE_OP_NEVER,
    VkCompareOp::VK_COMPARE_OP_ALWAYS,

    VkCompareOp::VK_COMPARE_OP_LESS,
    VkCompareOp::VK_COMPARE_OP_GREATER,

    VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL,
    VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL,

    VkCompareOp::VK_COMPARE_OP_NOT_EQUAL,
    VkCompareOp::VK_COMPARE_OP_EQUAL,
};
#endif
