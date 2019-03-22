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

#include "ResourceList.h"

#include "RenderDevice.h"
#include "CommandList.h"

#include "Buffer.h"
#include "Sampler.h"

#include <d3d11.h>

using namespace nya::rendering;

ResourceList& RenderDevice::allocateResourceList( const ResourceListDesc& description ) const
{
    const size_t resListIdx = renderContext->resListPoolIndex;

    ++renderContext->resListPoolIndex;
    if ( renderContext->resListPoolIndex >= renderContext->resListPoolCapacity ) {
        renderContext->resListPoolIndex = 0;
    }

    ResourceList& resList = renderContext->resListPool[resListIdx];
    resList = { 0 };

    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& sampler = description.samplers[i];

        if ( sampler.stageBind & SHADER_STAGE_VERTEX ) {
            resList.samplers.vertexStage[sampler.bindPoint] = sampler.resource->samplerState;
        }

        if ( sampler.stageBind & SHADER_STAGE_TESSELATION_CONTROL ) {
            resList.samplers.hullStage[sampler.bindPoint] = sampler.resource->samplerState;
        }
        
        if ( sampler.stageBind & SHADER_STAGE_TESSELATION_EVALUATION ) {
            resList.samplers.domainStage[sampler.bindPoint] = sampler.resource->samplerState;
        }
        
        if ( sampler.stageBind & SHADER_STAGE_PIXEL ) {
            resList.samplers.pixelStage[sampler.bindPoint] = sampler.resource->samplerState;
        }
        
        if ( sampler.stageBind & SHADER_STAGE_COMPUTE ) {
            resList.samplers.computeStage[sampler.bindPoint] = sampler.resource->samplerState;
        }
    }

    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& constantBuffer = description.constantBuffers[i];

        if ( constantBuffer.stageBind & SHADER_STAGE_VERTEX ) {
            resList.constantBuffers.vertexStage[constantBuffer.bindPoint] = constantBuffer.resource->bufferObject;
        }

        if ( constantBuffer.stageBind & SHADER_STAGE_TESSELATION_CONTROL ) {
            resList.constantBuffers.hullStage[constantBuffer.bindPoint] = constantBuffer.resource->bufferObject;
        }

        if ( constantBuffer.stageBind & SHADER_STAGE_TESSELATION_EVALUATION ) {
            resList.constantBuffers.domainStage[constantBuffer.bindPoint] = constantBuffer.resource->bufferObject;
        }

        if ( constantBuffer.stageBind & SHADER_STAGE_PIXEL ) {
            resList.constantBuffers.pixelStage[constantBuffer.bindPoint] = constantBuffer.resource->bufferObject;
        }

        if ( constantBuffer.stageBind & SHADER_STAGE_COMPUTE ) {
            resList.constantBuffers.computeStage[constantBuffer.bindPoint] = constantBuffer.resource->bufferObject;
        }
    }

    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& buffer = description.buffers[i];

        if ( buffer.stageBind & SHADER_STAGE_VERTEX ) {
            resList.buffers.vertexStage[buffer.bindPoint] = buffer.resource->bufferResourceView;
        }

        if ( buffer.stageBind & SHADER_STAGE_TESSELATION_CONTROL ) {
            resList.buffers.hullStage[buffer.bindPoint] = buffer.resource->bufferResourceView;
        }

        if ( buffer.stageBind & SHADER_STAGE_TESSELATION_EVALUATION ) {
            resList.buffers.domainStage[buffer.bindPoint] = buffer.resource->bufferResourceView;
        }

        if ( buffer.stageBind & SHADER_STAGE_PIXEL ) {
            resList.buffers.pixelStage[buffer.bindPoint] = buffer.resource->bufferResourceView;
        }

        if ( buffer.stageBind & SHADER_STAGE_COMPUTE ) {
            resList.buffers.computeStage[buffer.bindPoint] = buffer.resource->bufferResourceView;
        }
    }
    
    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& buffer = description.uavBuffers[i];

        if ( buffer.stageBind & eShaderStage::SHADER_STAGE_COMPUTE ) {
            resList.uavBuffers[buffer.bindPoint] = buffer.resource->bufferUAVObject;
            resList.uavBuffersBindCount++;
        }
    }

    return resList;
}

void CommandList::bindResourceList( ResourceList* resourceList )
{
    // For extra-safety, unbind SRV resources prior to UAV bindings
    if ( resourceList->uavBuffersBindCount != 0 ) {
        static constexpr ID3D11ShaderResourceView* NO_SRV[14] = { ( ID3D11ShaderResourceView* )nullptr };

        CommandListObject->deferredContext->VSSetShaderResources( 8, 14, NO_SRV );
        CommandListObject->deferredContext->HSSetShaderResources( 8, 14, NO_SRV );
        CommandListObject->deferredContext->DSSetShaderResources( 8, 14, NO_SRV );
        CommandListObject->deferredContext->PSSetShaderResources( 8, 14, NO_SRV );
        CommandListObject->deferredContext->CSSetShaderResources( 8, 14, NO_SRV );
    }

    CommandListObject->deferredContext->CSSetUnorderedAccessViews( 0, 7, resourceList->uavBuffers, nullptr );

    CommandListObject->deferredContext->VSSetSamplers( 0, 16, resourceList->samplers.vertexStage );
    CommandListObject->deferredContext->HSSetSamplers( 0, 16, resourceList->samplers.hullStage );
    CommandListObject->deferredContext->DSSetSamplers( 0, 16, resourceList->samplers.domainStage );
    CommandListObject->deferredContext->PSSetSamplers( 0, 16, resourceList->samplers.pixelStage );
    CommandListObject->deferredContext->CSSetSamplers( 0, 16, resourceList->samplers.computeStage );

    CommandListObject->deferredContext->VSSetConstantBuffers( 0, 14, resourceList->constantBuffers.vertexStage );
    CommandListObject->deferredContext->HSSetConstantBuffers( 0, 14, resourceList->constantBuffers.hullStage );
    CommandListObject->deferredContext->DSSetConstantBuffers( 0, 14, resourceList->constantBuffers.domainStage );
    CommandListObject->deferredContext->PSSetConstantBuffers( 0, 14, resourceList->constantBuffers.pixelStage );
    CommandListObject->deferredContext->CSSetConstantBuffers( 0, 14, resourceList->constantBuffers.computeStage );

    CommandListObject->deferredContext->VSSetShaderResources( 8, 14, resourceList->buffers.vertexStage );
    CommandListObject->deferredContext->HSSetShaderResources( 8, 14, resourceList->buffers.hullStage );
    CommandListObject->deferredContext->DSSetShaderResources( 8, 14, resourceList->buffers.domainStage );
    CommandListObject->deferredContext->PSSetShaderResources( 8, 14, resourceList->buffers.pixelStage );
    CommandListObject->deferredContext->CSSetShaderResources( 8, 14, resourceList->buffers.computeStage );
}
#endif
