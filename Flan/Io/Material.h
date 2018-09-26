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

#include <unordered_map>

class FileSystemObject;

struct MaterialRendererSetup
{
    bool    UseAlphaTest;
    bool    IsDoubleFace;
    bool    UseAlphaBlend;
};

struct CompiledMaterialLoadData
{
    fnString_t                                      MaterialName;
    fnString_t                                      ShaderFileName;
    std::unordered_map<unsigned int, fnString_t>    Textures;
    fnString_t                                      DepthShaderFileName;
    std::unordered_map<unsigned int, fnString_t>    DepthTextures;
    MaterialRendererSetup                           RendererSetup;
};

namespace flan
{
    namespace core
    {
        void    LoadCompiledMaterialFile( FileSystemObject* file, CompiledMaterialLoadData& data );
    }
}
