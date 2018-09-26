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
#include "BlendState.h"

#include "CommandList.h"
#include "RenderDevice.h"

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#include "OpenGL460/BlendState.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#include "Direct3D11/CommandList.h"
#include "Direct3D11/BlendState.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#include "Vulkan/CommandList.h"
#include "Vulkan/BlendState.h"
#endif

BlendState::BlendState()
    : blendStateDescription{}
    , nativeBlendStateObject( nullptr )
{

}

BlendState::BlendState( BlendState& state )
    : blendStateDescription( state.blendStateDescription )
    , nativeBlendStateObject( state.nativeBlendStateObject.get() )
{

}

BlendState& BlendState::operator = ( BlendState& state )
{
    blendStateDescription = state.blendStateDescription;
    nativeBlendStateObject.reset( state.nativeBlendStateObject.get() );

    return *this;
}

BlendState::~BlendState()
{
    blendStateDescription = {};
}

void BlendState::create( RenderDevice* renderDevice, const BlendStateDesc& description )
{
    blendStateDescription = description;

    nativeBlendStateObject.reset( flan::rendering::CreateBlendStateImpl( renderDevice->getNativeRenderContext(), blendStateDescription ) );
}

void BlendState::destroy( RenderDevice* renderDevice )
{
    flan::rendering::DestroyBlendStateImpl( renderDevice->getNativeRenderContext(), nativeBlendStateObject.get() );
}

void BlendState::bind( CommandList* cmdList )
{
    flan::rendering::BindBlendStateCmdImpl( cmdList->getNativeCommandList(), nativeBlendStateObject.get() );
}

const BlendStateDesc& BlendState::getDescription() const
{
    return blendStateDescription;
}

const BlendStateDesc::Key BlendState::getKey() const
{
    return blendStateDescription.stateKey;
}

BlendStateDesc::KeyValue BlendState::getKeyValue() const
{
    return blendStateDescription.stateKeyValue;
}

NativeBlendStateObject* BlendState::getNativeBlendStateObject() const
{
    return nativeBlendStateObject.get();
}
