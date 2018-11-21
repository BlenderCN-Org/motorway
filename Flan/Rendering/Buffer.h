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
struct NativeBufferObject;

#include <Rendering/ImageFormat.h>
#include <Rendering/ShaderStages.h>

struct BufferDesc
{
    enum BufferType : uint32_t
    {
        UNKNOWN = 0,
        CONSTANT_BUFFER,

        VERTEX_BUFFER,
        DYNAMIC_VERTEX_BUFFER,

        INDICE_BUFFER,
        DYNAMIC_INDICE_BUFFER,

        UNORDERED_ACCESS_VIEW_BUFFER,
        UNORDERED_ACCESS_VIEW_TEXTURE_1D,
        UNORDERED_ACCESS_VIEW_TEXTURE_2D,
        UNORDERED_ACCESS_VIEW_TEXTURE_3D,

        STRUCTURED_BUFFER,
        APPEND_STRUCTURED_BUFFER,
    } Type;

    eImageFormat    ViewFormat; // Required by UAV only
    uint32_t        Stride; // Required by vbo/ibo only

    union
    {
        struct
        {
            std::size_t     Size;
            std::size_t     SingleElementSize;
        };

        // UAV Textures
        struct
        {
            uint32_t        Width;
            uint32_t        Height;
            uint32_t        Depth;
            uint32_t        MipCount;
        };
    };
};

class Buffer
{
public:
    enum class BindMode
    {
        NONE,
        READ_ONLY,
        WRITE_ONLY,
    };

public:
                Buffer();
                Buffer( Buffer& ) = delete;
                Buffer& operator = ( Buffer& ) = delete;
                ~Buffer();

    void        create( RenderDevice* renderDevice, const BufferDesc& description, void* initialData = nullptr );
    void        destroy( RenderDevice* renderDevice );

    void        bind( CommandList* cmdList, const uint32_t bindingIndex = 0, const uint32_t shaderStagesToBindTo = eShaderStage::SHADER_STAGE_ALL );
    void        bindReadOnly( CommandList* cmdList, const uint32_t bindingIndex = 0, const uint32_t shaderStagesToBindTo = eShaderStage::SHADER_STAGE_ALL );
    void        unbind( CommandList* cmdList );

    void        update( RenderDevice* renderDevice, const void* dataToUpload, const std::size_t dataToUploadSize );
    void        updateAsynchronous( CommandList* cmdList, const void* dataToUpload, const std::size_t dataToUploadSize );
    void        updateRange( RenderDevice* renderDevice, const void* dataToUpload, const std::size_t dataToUploadSize, const int32_t bufferOffset = 0, const bool flushBuffer = true );

    const BufferDesc&   getDescription() const;
    NativeBufferObject* getNativeBufferObject() const;

private:
    BufferDesc                          bufferDescription;
    std::unique_ptr<NativeBufferObject> nativeBufferObject;

    uint32_t                            shaderStagesBindTo;
    uint32_t                            bindingIndex;
    BindMode                            bindMode;
};
