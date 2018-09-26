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
#include "CommandListPool.h"

#if FLAN_D3D11
#include "Direct3D11/CommandListPool.h"
#elif FLAN_GL460
#include "OpenGL460/CommandListPool.h"
#elif FLAN_VULKAN
#include "Vulkan/CommandListPool.h"
#endif

#include "RenderDevice.h"

CommandListPool::CommandListPool()
    : nativeCommandListPool( nullptr )
    , queueType( CommandListPool::GRAPHICS_QUEUE )
{

}

CommandListPool::~CommandListPool()
{

}

void CommandListPool::create( RenderDevice* renderDevice, const int poolCapacity, const CommandListPool::eQueueType poolDeviceQueue )
{
    nativeCommandListPool.reset( flan::rendering::CreateCommandListPoolImpl( renderDevice, poolCapacity, poolDeviceQueue ) );
}

void CommandListPool::destroy( RenderDevice* renderDevice )
{
    flan::rendering::DestroyCommandListPoolImpl( renderDevice, nativeCommandListPool.get() );
}

CommandList* CommandListPool::allocateCmdList( RenderDevice* renderDevice ) const
{
    return flan::rendering::AllocateCommandListPoolImpl( renderDevice, nativeCommandListPool.get() );
}
