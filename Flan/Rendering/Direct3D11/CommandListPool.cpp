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

#if FLAN_D3D11
#include "CommandListPool.h"

#include <d3d11.h>
#include <Rendering/CommandList.h>
#include "CommandList.h"
#include <Rendering/RenderDevice.h>

NativeCommandListPool* flan::rendering::CreateCommandListPoolImpl( RenderDevice* renderDevice, const int poolCapacity, const CommandListPool::eQueueType poolDeviceQueue )
{
    NativeCommandListPool* cmdListPool = new NativeCommandListPool();
    cmdListPool->cmdLists = new CommandList[poolCapacity]();
    cmdListPool->cmdListCount = poolCapacity;
    cmdListPool->availableIndex = 0;

    for ( int cmdListIdx = 0; cmdListIdx < cmdListPool->cmdListCount; cmdListIdx++ ) {
        cmdListPool->cmdLists[cmdListIdx].setNativeCommandList( flan::rendering::CreateCommandListImpl( renderDevice->getNativeRenderContext() ) );
    }

    return cmdListPool;
}

void flan::rendering::DestroyCommandListPoolImpl( RenderDevice* renderDevice, NativeCommandListPool* cmdListPool )
{
    cmdListPool->availableIndex = 0;

    for ( int cmdListIdx = 0; cmdListIdx < cmdListPool->cmdListCount; cmdListIdx++ ) {
        flan::rendering::DestroyCommandListImpl( renderDevice->getNativeRenderContext(), cmdListPool->cmdLists[cmdListIdx].getNativeCommandList() );
    }

    delete[] cmdListPool->cmdLists;

    cmdListPool->cmdListCount = 0;
}

CommandList* flan::rendering::AllocateCommandListPoolImpl( RenderDevice* renderDevice, NativeCommandListPool* cmdListPool )
{
    const auto cmdListIdx = cmdListPool->availableIndex;

    ++cmdListPool->availableIndex;
    if ( cmdListPool->availableIndex >= cmdListPool->cmdListCount ) {
        cmdListPool->availableIndex = 0;
    }

    return &cmdListPool->cmdLists[cmdListIdx];
}
#endif
