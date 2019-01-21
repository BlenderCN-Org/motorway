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

#if NYA_D3D11
#include <Rendering/RenderDevice.h>
#include "RenderDevice.h"
#include "Shader.h"

#include <d3d11.h>

Shader* RenderDevice::createShader( const eShaderStage stage, const void* bytecode, const size_t bytecodeSize )
{
    ID3D11Device* nativeDevice = renderContext->nativeDevice;
    HRESULT operationResult = S_OK;

    Shader* shader = nya::core::allocate<Shader>( memoryAllocator );
    switch ( stage ) {
    case SHADER_STAGE_VERTEX:
        operationResult = nativeDevice->CreateVertexShader( bytecode, bytecodeSize, NULL, &shader->vertexShader );
        
        // Needed for input layout creation
        shader->bytecode = nya::core::allocateArray<uint8_t>( memoryAllocator, bytecodeSize );
        shader->bytecodeSize = bytecodeSize;

        memcpy( shader->bytecode, bytecode, bytecodeSize * sizeof( uint8_t ) );
        break;

    case SHADER_STAGE_PIXEL:
        operationResult = nativeDevice->CreatePixelShader( bytecode, bytecodeSize, NULL, &shader->pixelShader );
        break;

    case SHADER_STAGE_TESSELATION_CONTROL:
        operationResult = nativeDevice->CreateHullShader( bytecode, bytecodeSize, NULL, &shader->tesselationControlShader );
        break;

    case SHADER_STAGE_TESSELATION_EVALUATION:
        operationResult = nativeDevice->CreateDomainShader( bytecode, bytecodeSize, NULL, &shader->tesselationEvalShader );
        break;

    case SHADER_STAGE_COMPUTE:
        operationResult = nativeDevice->CreateComputeShader( bytecode, bytecodeSize, NULL, &shader->computeShader );
        break;
    }

    if ( operationResult != S_OK ) {
        NYA_CERR << "Failed to load precompiled shader (error code: " << NYA_PRINT_HEX( operationResult ) << ")" << std::endl;
        return nullptr;
    }

    return shader;
}

void RenderDevice::destroyShader( Shader* shader )
{
    shader->vertexShader->Release();
    nya::core::free( memoryAllocator, shader );
}
#endif
