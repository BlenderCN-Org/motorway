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

#if FLAN_D3D11
#include <Rendering/Shader.h>
#include <Rendering/ShaderStages.h>

#include <d3d11.h>

struct NativeRenderContext;

struct NativeShaderObject
{
    union
    {
        ID3D11Resource*         resource;
        ID3D11VertexShader*     vertexShader;
        ID3D11HullShader*       tesselationControlShader;
        ID3D11DomainShader*     tesselationEvalShader;
        ID3D11PixelShader*      pixelShader;
        ID3D11ComputeShader*    computeShader;
    };

    NativeShaderObject()
        : resource( nullptr )
    {

    }
};

namespace flan
{
    namespace rendering
    {
        NativeShaderObject* CreateAndCompileShaderImpl( NativeRenderContext* nativeRenderContext, const eShaderStage shaderStage, const char* shaderToBeCompiled, const std::size_t shaderToBeCompiledLength );
        NativeShaderObject* CreatePrecompiledShaderImpl( NativeRenderContext* nativeRenderContext, const eShaderStage shaderStage,  const uint8_t* precompiledShader, const std::size_t precompiledShaderLength );
        void DestroyShaderImpl( NativeRenderContext* nativeRenderContext, NativeShaderObject* ShaderObject );
    }
}
#endif
