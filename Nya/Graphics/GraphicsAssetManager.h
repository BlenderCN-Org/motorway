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
class ShaderCache;
class VirtualFileSystem;

class Material;
class Mesh;

struct Texture;
struct FontDescriptor;
struct Model;

class BaseAllocator;
class FreeListAllocator;

#include <map>

class GraphicsAssetManager
{
/*
public:
    struct RawTexels
    {
        ~RawTexels();

        uint16_t*   data;
        std::size_t bytePerPixel;
        int32_t     width;
        int32_t     height;
        int32_t     channelCount;
    };*/

public:
                    GraphicsAssetManager( BaseAllocator* allocator, RenderDevice* renderDevice, ShaderCache* shaderCache, VirtualFileSystem* virtualFileSystem );
                    GraphicsAssetManager( GraphicsAssetManager& ) = delete;
	                ~GraphicsAssetManager();

    void            destroy();

    Texture*        getTexture( const nyaChar_t* assetName, const bool forceReload = false );
    FontDescriptor* getFont( const nyaChar_t* assetName, const bool forceReload = false );
    /*Material*       getMaterialCopy( const nyaChar_t* assetName );
    Material*       getMaterial( const nyaChar_t* assetName, const bool forceReload = false );*/
    Mesh*           getMesh( const nyaChar_t* assetName, const bool forceReload = false );
    //Model*          getModel( const nyaChar_t* assetName, const bool forceReload = false );

    // WARNING (for now) you are responsible of releasing the memory (which is a bad thing)
    //void            getImageTexels( const nyaChar_t* assetName, GraphicsAssetManager::RawTexels& texels );
    
private:
    FreeListAllocator*      assetStreamingHeap;
    RenderDevice*           renderDevice;
    ShaderCache*            shaderCache;
    VirtualFileSystem*      virtualFileSystem;

    std::map<nyaStringHash_t, Material*>           materialMap;
    std::map<nyaStringHash_t, Mesh*>               meshMap;
    std::map<nyaStringHash_t, Model*>              modelMap;
    std::map<nyaStringHash_t, Texture*>            textureMap;
    std::map<nyaStringHash_t, FontDescriptor*>     fontMap;
};
