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
#include <Shared.h>

#if NYA_VULKAN
#include <Rendering/RenderDevice.h>
#include "RenderDevice.h"
#include "Shader.h"

#include <vulkan/vulkan.h>

VkShaderStageFlagBits GetVkStageFlags( const eShaderStage shaderStage )
{
    switch ( shaderStage ) {
    case SHADER_STAGE_VERTEX:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case SHADER_STAGE_TESSELATION_CONTROL:
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case SHADER_STAGE_TESSELATION_EVALUATION:
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case SHADER_STAGE_PIXEL:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case SHADER_STAGE_COMPUTE:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    default:
        return VK_SHADER_STAGE_ALL_GRAPHICS;
    }
}

Shader* RenderDevice::createShader( const eShaderStage stage, const void* bytecode, const size_t bytecodeSize )
{
    Shader* shader = nya::core::allocate<Shader>( memoryAllocator ); 

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0u;
    createInfo.codeSize = bytecodeSize;
    createInfo.pCode = reinterpret_cast< const uint32_t* >( bytecode );

    VkShaderModule shaderModule;
    VkResult operationResult = vkCreateShaderModule( renderContext->device, &createInfo, nullptr, &shaderModule );
    if ( operationResult != VK_SUCCESS ) {
        NYA_CERR << "Failed to load precompiled shader (error code: " << operationResult << ")" << std::endl;
        return nullptr;
    }

    shader->shaderModule = shaderModule;
    shader->shaderStage = GetVkStageFlags( stage );

    return shader;
}

void RenderDevice::destroyShader( Shader* shader )
{
    vkDestroyShaderModule( renderContext->device, shader->shaderModule, nullptr );
    nya::core::free( memoryAllocator, shader );
}
#endif
