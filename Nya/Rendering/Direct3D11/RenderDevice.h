/*
Project Motorway Source Code
Copyright (C) 2018 Pr�vost Baptiste

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

#if NYA_D3D11
struct ID3D11DeviceContext;
struct ID3D11Device;
struct IDXGISwapChain;
struct RenderTarget;

class PoolAllocator;
class CommandList;
struct ResourceList;

struct RenderContext
{
                            RenderContext();
                            ~RenderContext();

    ID3D11DeviceContext*	nativeDeviceContext;
    ID3D11Device*			nativeDevice;
    IDXGISwapChain*			swapChain;
    RenderTarget*           backbuffer;
    bool                    enableVsync;

    CommandList*            cmdListPool;
    size_t                  cmdListPoolIndex;
    size_t                  cmdListPoolCapacity;

    ResourceList*           resListPool;
    size_t                  resListPoolIndex;
    size_t                  resListPoolCapacity;

    PoolAllocator*          renderPassAllocator;
};
#endif
