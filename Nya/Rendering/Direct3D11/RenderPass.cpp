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

#include <Rendering/CommandList.h>
#include "CommandList.h"

#include "RenderTarget.h"
#include "Texture.h"

#include <Core/Allocators/PoolAllocator.h>

#include <d3d11.h>

struct RenderPass
{
    ID3D11RenderTargetView* renderTargetViews[8];
    ID3D11DepthStencilView* depthStencilView;

    FLOAT clearValue[8+1][4];
    bool clearTarget[8+1];
    UINT rtvCount;

    ID3D11ShaderResourceView* shaderResourceView[8];
    UINT srvCount;
};

RenderPass* RenderDevice::createRenderPass( const RenderPassDesc& description )
{
    RenderPass* renderPass = nya::core::allocate<RenderPass>( renderContext->renderPassAllocator );
    renderPass->rtvCount = 0u;
    renderPass->srvCount = 0u;

    for ( int i = 0; i < 8; i++ ) {
        switch ( description.attachements[i].bindMode ) {
        case RenderPassDesc::READ:
        {
            Texture* texture = ( description.attachements[i].targetState != RenderPassDesc::IS_TEXTURE ) 
                                ? description.attachements[i].renderTarget->texture 
                                : description.attachements[i].texture;

            renderPass->shaderResourceView[renderPass->srvCount++] = ( texture == nullptr ) ? nullptr : texture->shaderResourceView;
        } break;

        case RenderPassDesc::WRITE:
            renderPass->clearTarget[renderPass->rtvCount] = ( description.attachements[i].targetState != RenderPassDesc::DONT_CARE );
            memcpy( renderPass->clearValue, description.attachements[i].clearValue, sizeof( FLOAT ) * 4 );

            renderPass->renderTargetViews[renderPass->rtvCount++] = description.attachements[i].renderTarget->textureRenderTargetView;
            break;

        case RenderPassDesc::WRITE_DEPTH:
            renderPass->clearTarget[8] = ( description.attachements[i].targetState != RenderPassDesc::DONT_CARE );
            memcpy( renderPass->clearValue, description.attachements[i].clearValue, sizeof( FLOAT ) * 4 );

            renderPass->depthStencilView = description.attachements[i].renderTarget->textureDepthRenderTargetView;
            break;

        default:
            break;
        }
    }

    return renderPass;
}

void RenderDevice::destroyRenderPass( RenderPass* renderPass )
{
    nya::core::free( renderContext->renderPassAllocator, renderPass );
}

void CommandList::useRenderPass( RenderPass* renderPass )
{
    for ( UINT i = 0; i < renderPass->rtvCount; i++ ) {
        if ( renderPass->clearTarget[i] ) {
            NativeCommandList->deferredContext->ClearRenderTargetView( renderPass->renderTargetViews[i], renderPass->clearValue[i] );
        }
    }
    
    if ( renderPass->clearTarget[8] ) {
        NativeCommandList->deferredContext->ClearDepthStencilView( renderPass->depthStencilView, D3D11_CLEAR_DEPTH, renderPass->clearValue[8][0], 0x0 );
    }

    NativeCommandList->deferredContext->OMSetRenderTargets( renderPass->rtvCount, renderPass->renderTargetViews, renderPass->depthStencilView );
    NativeCommandList->deferredContext->PSSetShaderResources( 0, renderPass->srvCount, renderPass->shaderResourceView );
}
#endif
