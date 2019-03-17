/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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
struct VkRenderPass_T;
struct VkFramebuffer_T;
struct VkImageView_T;

struct RenderPass
{
    VkRenderPass_T*     renderPass;
    VkFramebuffer_T*    framebuffer;
    VkClearValue        clearValues[24];
    VkImageView_T*      imageViews[24];
    uint32_t            attachmentCount;
    uint32_t            width;
    uint32_t            height;
};
#endif
