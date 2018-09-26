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
#include "Shader.h"

#include "RenderContext.h"

static constexpr VkShaderStageFlagBits VK_SHADER_STAGE[eShaderStage::SHADER_STAGE_COUNT] = {
    VK_SHADER_STAGE_VERTEX_BIT,
    VK_SHADER_STAGE_FRAGMENT_BIT,
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    VK_SHADER_STAGE_COMPUTE_BIT,
};

NativeShaderObject* flan::rendering::CreateAndCompileShaderImpl( NativeRenderContext* nativeRenderContext, const eShaderStage shaderStage, const char* shaderToBeCompiled, const std::size_t shaderToBeCompiledLength )
{
    FLAN_CWARN << "Unsupported feature!" << std::endl;

    return nullptr;
}

NativeShaderObject* flan::rendering::CreatePrecompiledShaderImpl( NativeRenderContext* nativeRenderContext, const eShaderStage shaderStage,  const uint8_t* precompiledShader, const std::size_t precompiledShaderLength )
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = precompiledShaderLength;
    createInfo.pCode = reinterpret_cast< const uint32_t* >( precompiledShader );

    // Create shader module
    VkShaderModule shaderModule;
    VkResult operationResult = vkCreateShaderModule( nativeRenderContext->device, &createInfo, nullptr, &shaderModule );
    if ( operationResult != VK_SUCCESS ) {
        FLAN_CERR << "Failed to load precompiled shader (error code: " << operationResult << ")" << std::endl;
        return nullptr;
    }

    // Create stage infos (not used yet)
    VkPipelineShaderStageCreateInfo shaderStageInfos = {};
    shaderStageInfos.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos.stage = VK_SHADER_STAGE[shaderStage];
    shaderStageInfos.module = shaderModule;
    shaderStageInfos.pName = "main";

    // Create native object
    NativeShaderObject* nativeShaderObject = new NativeShaderObject();
    nativeShaderObject->nativeShaderModule = shaderModule;
    nativeShaderObject->nativeStageInfos = shaderStageInfos;
    
    return nativeShaderObject;
}

void flan::rendering::DestroyShaderImpl( NativeRenderContext* nativeRenderContext, NativeShaderObject* shaderObject )
{
    vkDestroyShaderModule( nativeRenderContext->device, shaderObject->nativeShaderModule, nullptr );
}
#endif
