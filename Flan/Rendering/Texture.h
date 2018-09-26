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
class CommandList;
struct NativeTextureObject;

#include "TextureDescription.h"
#include <Rendering/ShaderStages.h>
#include <vector>

class Texture
{
public:
                Texture();
                Texture( Texture& ) = delete;
                Texture& operator = ( Texture& ) = delete;
                ~Texture();

    void        createAsTexture1D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );
    void        createAsTexture2D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );
    void        createAsTexture3D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );

    void        destroy( RenderDevice* renderDevice );

    void        bind( CommandList* cmdList, const uint32_t bindingIndex = 0, const uint32_t shaderStagesToBindTo = eShaderStage::SHADER_STAGE_ALL );
    void        unbind( CommandList* cmdList );

    void                        setResourceName( RenderDevice* renderDevice, const std::string& name );
    const std::string&          getResourceName() const;
    const TextureDescription&   getDescription() const;
    const NativeTextureObject*  getNativeObject() const;

    void        retrieveTexelsLDR( RenderDevice* renderDevice, std::vector<uint8_t>& texels );
    void        retrieveTexelsHDR( RenderDevice* renderDevice, std::vector<float>& texels );

private:
    TextureDescription                   textureDescription;
    std::unique_ptr<NativeTextureObject> nativeTextureObject;

    std::string                          resourceName;
    uint32_t                             shaderStagesBindTo;
    uint32_t                             bindingIndex;
};
