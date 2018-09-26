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
#include "PipelineState.h"

#include "RenderDevice.h"

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#include "OpenGL460/PipelineState.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#include "Direct3D11/PipelineState.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#include "Vulkan/PipelineState.h"
#endif

PipelineState::PipelineState()
    : pipelineStateDescription{}
    , nativePipelineStateObject( nullptr )
{

}

PipelineState::~PipelineState()
{
    pipelineStateDescription = {};
}

void PipelineState::create( RenderDevice* renderDevice, const PipelineStateDesc& description )
{
    nativePipelineStateObject.reset( flan::rendering::CreatePipelineStateImpl( renderDevice->getNativeRenderContext(), description ) );

    pipelineStateDescription = description;
}

void PipelineState::destroy( RenderDevice* renderDevice )
{
    flan::rendering::DestroyPipelineStateImpl( renderDevice->getNativeRenderContext(), nativePipelineStateObject.get() );
}

NativePipelineStateObject* PipelineState::getNativePipelineStateObject() const
{
    return nativePipelineStateObject.get();
}

const PipelineStateDesc* PipelineState::getDescription() const
{
    return &pipelineStateDescription;
}
