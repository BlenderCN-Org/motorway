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

#if NYA_D3D11
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include "RenderDevice.h"
#include "CommandList.h"

#include "Buffer.h"
#include "Sampler.h"
#include "PipelineState.h"
#include "Texture.h"
#include "RenderTarget.h"

#include <d3d11.h>

using namespace nya::rendering;

void RenderDevice::updateResourceList( PipelineState* pipelineState, const ResourceList& resourceList )
{
    for ( int i = 0; i < pipelineState->resourceList.resourceToBindCount; i++ ) {
        auto& resource = pipelineState->resourceList.resources[i];
        
        if ( resourceList.resource[resource.resourceIndex].buffer == nullptr ) {
            continue;
        }

        switch ( resource.type ) {
        case PipelineState::ResourceListLayout::CBuffer:
            *resource.buffers = resourceList.resource[resource.resourceIndex].buffer->bufferObject;
            break;
        case PipelineState::ResourceListLayout::Buffer:
            *resource.shaderResourceView = resourceList.resource[resource.resourceIndex].buffer->bufferResourceView;
            break;
        case PipelineState::ResourceListLayout::Sampler:
            *resource.samplerState = resourceList.resource[resource.resourceIndex].sampler->samplerState;
            break;
        case PipelineState::ResourceListLayout::Texture:
            *resource.shaderResourceView = resourceList.resource[resource.resourceIndex].texture->shaderResourceView;
            break;
        case PipelineState::ResourceListLayout::RenderTarget:
            *resource.shaderResourceView = resourceList.resource[resource.resourceIndex].renderTarget->texture->shaderResourceView;
            break;
        case PipelineState::ResourceListLayout::UAVResource:
            *resource.unorderedAccessView = resourceList.resource[resource.resourceIndex].buffer->bufferUAVObject;
            break;
        default:
            break;
        }
    }
}
#endif
