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

#include <Rendering/ShaderStages.h>

#include <unordered_map>
#include <memory>

class RenderDevice;
class FileSystemObject;
class VirtualFileSystem;
class Shader;

class ShaderStageManager
{
public:
                            ShaderStageManager( RenderDevice* activeRenderDevice, VirtualFileSystem* activeVFS );
                            ShaderStageManager( ShaderStageManager& ) = delete;
                            ~ShaderStageManager();
	
    Shader*                 getOrUploadStage( const fnString_t& shaderFilename, const eShaderStage stageType, const bool forceReload = false );

private:
    VirtualFileSystem*                                          virtualFileSystem;
    RenderDevice*                                               renderDevice;

	std::unordered_map<fnStringHash_t, std::unique_ptr<Shader>>	cachedStages;
};
