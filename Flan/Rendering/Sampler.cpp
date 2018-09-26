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
#include "Sampler.h"

#include "RenderDevice.h"
#include "CommandList.h"

#if FLAN_GL460
#include "OpenGL460/RenderContext.h"
#include "OpenGL460/Sampler.h"
#elif FLAN_D3D11
#include "Direct3D11/RenderContext.h"
#include "Direct3D11/CommandList.h"
#include "Direct3D11/Sampler.h"
#elif FLAN_VULKAN
#include "Vulkan/RenderContext.h"
#include "Vulkan/CommandList.h"
#include "Vulkan/Sampler.h"
#endif

Sampler::Sampler()
    : samplerDescription{}
    , nativeSamplerObject( nullptr )
{

}

Sampler::~Sampler()
{

}

void Sampler::create( RenderDevice* renderDevice, const SamplerDesc& description )
{
    samplerDescription = description;

    nativeSamplerObject.reset( flan::rendering::CreateSamplerImpl( renderDevice->getNativeRenderContext(), samplerDescription ) );
}

void Sampler::destroy( RenderDevice* renderDevice )
{
    flan::rendering::DestroySamplerImpl( renderDevice->getNativeRenderContext(), nativeSamplerObject.get() );
}

void Sampler::bind( CommandList* cmdList, const uint32_t bindingIndex )
{
    flan::rendering::BindSamplerCmdImpl( cmdList->getNativeCommandList(), nativeSamplerObject.get(), bindingIndex );
}

const SamplerDesc* Sampler::getDescription() const
{
    return &samplerDescription;
}
