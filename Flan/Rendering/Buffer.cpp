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
#include "Buffer.h"

#include "RenderDevice.h"
#include "CommandList.h"

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#include "OpenGL460/Buffer.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#include "Direct3D11/CommandList.h"
#include "Direct3D11/Buffer.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#include "Vulkan/CommandList.h"
#include "Vulkan/Buffer.h"
#endif

Buffer::Buffer()
    : bufferDescription{}
    , nativeBufferObject( nullptr )
    , shaderStagesBindTo( 0 )
    , bindingIndex( 0 )
    , bindMode( Buffer::BindMode::NONE )
{

}

Buffer::~Buffer()
{
    bufferDescription = {};

    shaderStagesBindTo = 0;
    bindingIndex = 0;
    bindMode = BindMode::NONE;
}

void Buffer::create( RenderDevice* renderDevice, const BufferDesc& description, void* initialData )
{
    bufferDescription = description;

    nativeBufferObject.reset( flan::rendering::CreateBufferImpl( renderDevice->getNativeRenderContext(), bufferDescription, initialData ) );
}

void Buffer::destroy( RenderDevice* renderDevice )
{
    flan::rendering::DestroyBufferImpl( renderDevice->getNativeRenderContext(), nativeBufferObject.get() );
}

void Buffer::bind( CommandList* cmdList, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
    this->shaderStagesBindTo = shaderStagesToBindTo;
    this->bindingIndex = bindingIndex;
    this->bindMode = BindMode::WRITE_ONLY;

    flan::rendering::BindBufferCmdImpl( cmdList->getNativeCommandList(), nativeBufferObject.get(), shaderStagesToBindTo, bindingIndex );
}

void Buffer::bindReadOnly( CommandList* cmdList, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
    this->shaderStagesBindTo = shaderStagesToBindTo;
    this->bindingIndex = bindingIndex;
    this->bindMode = BindMode::READ_ONLY;

    flan::rendering::BindBufferReadOnlyCmdImpl( cmdList->getNativeCommandList(), nativeBufferObject.get(), shaderStagesToBindTo, bindingIndex );
}

void Buffer::unbind( CommandList* cmdList )
{
    if ( this->shaderStagesBindTo == 0 ) {
        return;
    }

    flan::rendering::UnbindBufferCmdImpl( cmdList->getNativeCommandList(), nativeBufferObject.get(), this->shaderStagesBindTo, this->bindingIndex, this->bindMode );

    this->shaderStagesBindTo = 0;
    this->bindingIndex = 0;
    this->bindMode = BindMode::NONE;
}

void Buffer::update( RenderDevice* renderDevice, const void* dataToUpload, const std::size_t dataToUploadSize )
{
    flan::rendering::UpdateBufferImpl( renderDevice->getNativeRenderContext(), nativeBufferObject.get(), dataToUpload, dataToUploadSize );
}

void Buffer::updateAsynchronous( CommandList* cmdList, const void* dataToUpload, const std::size_t dataToUploadSize )
{
    flan::rendering::UpdateBufferAsynchronousImpl( cmdList->getNativeCommandList(), nativeBufferObject.get(), dataToUpload, dataToUploadSize );
}

void Buffer::updateRange( RenderDevice* renderDevice, const void* dataToUpload, const std::size_t dataToUploadSize, const int32_t bufferOffset, const bool flushBuffer )
{
    if ( flushBuffer ) {
        flan::rendering::FlushAndUpdateBufferRangeImpl( renderDevice->getNativeRenderContext(), nativeBufferObject.get(), dataToUpload, dataToUploadSize, bufferOffset );
    } else {
        flan::rendering::UpdateBufferRangeImpl( renderDevice->getNativeRenderContext(), nativeBufferObject.get(), dataToUpload, dataToUploadSize, bufferOffset );
    }
}

const BufferDesc& Buffer::getDescription() const
{
    return bufferDescription;
}

NativeBufferObject* Buffer::getNativeBufferObject() const
{
    return nativeBufferObject.get();
}
