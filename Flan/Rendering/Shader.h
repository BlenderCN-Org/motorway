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

class RenderDevice;
struct NativeShaderObject;

#include "ShaderStages.h"
#include <vector>

class Shader
{
public:
                Shader( const eShaderStage shaderStage );
                Shader( Shader& ) = delete;
                Shader& operator = ( Shader& ) = delete;
                ~Shader();

    void        create( RenderDevice* renderDevice, const char* shaderToBeCompiled, const std::size_t shaderToBeCompiledLength );
    void        create( RenderDevice* renderDevice, const uint8_t* precompiledShader, const std::size_t precompiledShaderLength );
    void        destroy( RenderDevice* renderDevice );

    NativeShaderObject* getNativeShaderObject() const;
    const std::vector<uint8_t>* getShaderBytecode() const;

private:
    std::unique_ptr<NativeShaderObject> nativeShaderObject;
    eShaderStage                        stage;
    std::vector<uint8_t>                shaderBytecode;
};
