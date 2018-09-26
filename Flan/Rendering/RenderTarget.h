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
struct NativeRenderTargetObject;

#include <Rendering/ShaderStages.h>
#include "TextureDescription.h"
#include <vector>

class RenderTarget
{
public:
                RenderTarget();
                RenderTarget( RenderTarget& ) = delete;
                RenderTarget& operator = ( RenderTarget& ) = delete;
                ~RenderTarget();

    void        createAsRenderTarget1D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );
    void        createAsRenderTarget2D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );
    void        createAsRenderTarget3D( RenderDevice* renderDevice, const TextureDescription& description, void* initialData = nullptr, const std::size_t initialDataSize = 0 );

    void        destroy( RenderDevice* renderDevice );

    void        bind( CommandList* cmdList, const uint32_t bindingIndex = 0, const uint32_t shaderStagesToBindTo = eShaderStage::SHADER_STAGE_ALL );
    void        unbind( CommandList* cmdList );

    TextureDescription::Dimension getTextureDimension() const;
    NativeTextureObject* getNativeTextureObject() const;
    NativeRenderTargetObject* getNativeRenderTargetObject() const;

    const TextureDescription& getDescription() const;

    void        retrieveTexelsLDR( RenderDevice* renderDevice, std::vector<uint8_t>& texels );
    void        retrieveLayerTexelsLDR( RenderDevice* renderDevice, const unsigned int layerIndex, const unsigned int mipLevel, std::vector<uint8_t>& texels );

    void        retrieveTexelsHDR( RenderDevice* renderDevice, std::vector<float>& texels );
    void        retrieveLayerTexelsHDR( RenderDevice* renderDevice, const unsigned int layerIndex, std::vector<float>& texels );

private:
    TextureDescription                          textureDescription;
    std::unique_ptr<NativeTextureObject>        nativeTextureObject;
    std::unique_ptr<NativeRenderTargetObject>   nativeRenderTargetObject;
    uint32_t                                    shaderStagesBindTo;
    uint32_t                                    bindingIndex;
};
