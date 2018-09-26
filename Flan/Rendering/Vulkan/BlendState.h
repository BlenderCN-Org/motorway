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

#if FLAN_VULKAN
#include <Rendering/BlendState.h>
#include <vulkan/vulkan.hpp>

struct NativeRenderContext;
struct NativeCommandList;

struct NativeBlendStateObject
{
    VkPipelineColorBlendStateCreateInfo nativeBlendState;
};

static_assert( std::is_pod<NativeBlendStateObject>(), "NativeBlendStateObject: not a POD!" );

namespace flan
{
    namespace rendering
    {
        NativeBlendStateObject* CreateBlendStateImpl( NativeRenderContext* nativeRenderContext, const BlendStateDesc& description );
        void DestroyBlendStateImpl( NativeRenderContext* nativeRenderContext, NativeBlendStateObject* blendStateObject );
        void BindBlendStateCmdImpl( NativeCommandList* nativeCmdList, NativeBlendStateObject* blendStateObject );
    }
}
#endif
