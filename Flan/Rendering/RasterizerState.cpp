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
#include "RasterizerState.h"

#include "RenderDevice.h"
#include "CommandList.h"

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#include "OpenGL460/RasterizerState.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#include "Direct3D11/CommandList.h"
#include "Direct3D11/RasterizerState.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#include "Vulkan/CommandList.h"
#include "Vulkan/RasterizerState.h"
#endif

RasterizerState::RasterizerState()
    : rasterizerStateDescription{}
    , nativeRasterizerStateObject( nullptr )
{

}

RasterizerState::~RasterizerState()
{
    rasterizerStateDescription = {};
}

void RasterizerState::create( RenderDevice* renderDevice, const RasterizerStateDesc& description )
{
    rasterizerStateDescription = description;

    nativeRasterizerStateObject.reset( flan::rendering::CreateRasterizerStateImpl( renderDevice->getNativeRenderContext(), rasterizerStateDescription ) );
}

void RasterizerState::destroy( RenderDevice* renderDevice )
{
    flan::rendering::DestroyRasterizerStateImpl( renderDevice->getNativeRenderContext(), nativeRasterizerStateObject.get() );
}

void RasterizerState::bind( CommandList* cmdList )
{
    flan::rendering::BindRasterizerStateCmdImpl( cmdList->getNativeCommandList(), nativeRasterizerStateObject.get() );
}

const RasterizerStateDesc& RasterizerState::getDescription() const
{
    return rasterizerStateDescription;
}

const RasterizerStateDesc::Key RasterizerState::getKey() const
{
    return rasterizerStateDescription.stateKey;
}

RasterizerStateDesc::KeyValue RasterizerState::getKeyValue() const
{
    return rasterizerStateDescription.stateKeyValue;
}

NativeRasterizerStateObject* RasterizerState::getNativeRasterizerStateObject() const
{
    return nativeRasterizerStateObject.get();
}
