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
class ShaderStageManager;
class VirtualFileSystem;
class Texture;
class Material;
class Mesh;
struct FontDescriptor;
struct Model;

#include <unordered_map>
#include <vector>

class GraphicsAssetManager
{
public:
                    GraphicsAssetManager( RenderDevice* activeRenderDevice, ShaderStageManager* activeShaderStageManager, VirtualFileSystem* activeVFS );
                    GraphicsAssetManager( GraphicsAssetManager& ) = delete;
	                ~GraphicsAssetManager();

    void            destroy();

    Texture*        getTexture( const fnChar_t* assetName, const bool forceReload = false );
    FontDescriptor* getFont( const fnChar_t* assetName, const bool forceReload = false );
    Material*       getMaterial( const fnChar_t* assetName, const bool forceReload = false );
    Mesh*           getMesh( const fnChar_t* assetName, const bool forceReload = false );
    Model*          getModel( const fnChar_t* assetName, const bool forceReload = false );

private:
    RenderDevice*       renderDevice;
    ShaderStageManager* shaderStageManager;
    VirtualFileSystem*  virtualFileSystem;

    std::unordered_map<fnStringHash_t, std::unique_ptr<Material>>       materialMap;
    std::unordered_map<fnStringHash_t, std::unique_ptr<Mesh>>           meshMap;
    std::unordered_map<fnStringHash_t, std::unique_ptr<Model>>          modelMap;
    std::unordered_map<fnStringHash_t, std::unique_ptr<Texture>>        textureMap;
    std::unordered_map<fnStringHash_t, std::unique_ptr<FontDescriptor>> fontMap;
};
