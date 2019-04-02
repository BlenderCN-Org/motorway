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
#include "RenderDevice.h"
#include "PipelineState.h"

#include <Rendering/CommandList.h>
#include "CommandList.h"

#include "RenderTarget.h"
#include "Texture.h"
#include "Buffer.h"

#include <Core/Allocators/PoolAllocator.h>

void CommandList::bindRenderPass( PipelineState* pipelineState, const RenderPass& renderPass )
{
    for ( int i = 0; i < pipelineState->renderPass.resourceToBindCount; i++ ) {
        auto& resource = pipelineState->renderPass.resources[i];

        if ( renderPass.attachement[resource.resourceIndex].renderTarget == nullptr ) {
            continue;
        }

        switch ( resource.type ) {
        case PipelineState::RenderPassLayout::RenderTargetView:
            *resource.renderTargetView = renderPass.attachement[resource.resourceIndex].renderTarget->textureRenderTargetView;
            break;
        case PipelineState::RenderPassLayout::DepthStencilView:
            *resource.depthStencilView = renderPass.attachement[resource.resourceIndex].renderTarget->textureDepthRenderTargetView;
            break;
        case PipelineState::RenderPassLayout::ShaderResourceView:
            *resource.shaderResourceView = renderPass.attachement[resource.resourceIndex].renderTarget->texture->shaderResourceView;
            break;
        default:
            break;
        }
    }

    auto& renderPassLayout = pipelineState->renderPass;
    for ( UINT i = 0; i < renderPassLayout.rtvCount; i++ ) {
        if ( renderPassLayout.clearTarget[i] ) {
            CommandListObject->deferredContext->ClearRenderTargetView( renderPassLayout.renderTargetViews[i], renderPassLayout.clearValue[i] );
        }
    }
    
    if ( renderPassLayout.clearTarget[8] ) {
        CommandListObject->deferredContext->ClearDepthStencilView( renderPassLayout.depthStencilView, D3D11_CLEAR_DEPTH, renderPassLayout.clearValue[8][0], 0x0 );
    }
    
    // TODO Avoid resource binding dependencies (skip explicit SRV unbind)?
    static constexpr ID3D11ShaderResourceView* NO_SRV[8] = { ( ID3D11ShaderResourceView* )nullptr };

    CommandListObject->deferredContext->PSSetShaderResources( 0, 8, NO_SRV );
    CommandListObject->deferredContext->CSSetShaderResources( 0, 8, NO_SRV );

    CommandListObject->deferredContext->OMSetRenderTargets( renderPassLayout.rtvCount, renderPassLayout.renderTargetViews, renderPassLayout.depthStencilView );

    CommandListObject->deferredContext->PSSetShaderResources( 0, 8, renderPassLayout.pixelStage.shaderResourceView );
    CommandListObject->deferredContext->CSSetShaderResources( 0, 8, renderPassLayout.computeStage.shaderResourceView );
}
#endif
