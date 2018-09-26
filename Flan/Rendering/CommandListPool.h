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

#include "Viewport.h"
#include "RenderTarget.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"
#include "PipelineState.h"

class RenderDevice;
class PipelineState;
struct NativeCommandList;
struct NativeCommandListPool;

class CommandListPool
{
public:
    enum eQueueType
    {
        GRAPHICS_QUEUE = 0,
        COMPUTE_QUEUE,
    };

public:
                            CommandListPool();
                            CommandListPool( CommandListPool& ) = delete;
                            CommandListPool& operator = ( CommandListPool& ) = delete;
                            ~CommandListPool();

    void                    create( RenderDevice* renderDevice, const int poolCapacity, const CommandListPool::eQueueType poolDeviceQueue = CommandListPool::GRAPHICS_QUEUE );
    void                    destroy( RenderDevice* renderDevice );

    CommandList*            allocateCmdList( RenderDevice* renderDevice ) const;

private:
    std::unique_ptr<NativeCommandListPool>  nativeCommandListPool;
    eQueueType                              queueType;
};
