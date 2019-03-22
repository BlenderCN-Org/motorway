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
#include "RenderPass.h"

#include <Rendering/CommandList.h>
#include "CommandList.h"

#include "RenderTarget.h"
#include "Texture.h"
#include "Buffer.h"

#include <Core/Allocators/PoolAllocator.h>

RenderPass* RenderDevice::createRenderPass( const RenderPassDesc& description )
{
    RenderPass* renderPass = nya::core::allocate<RenderPass>( renderContext->renderPassAllocator );
    renderPass->rtvCount = 0u;

    renderPass->pixelStage.srvCount = 0u;
    renderPass->computeStage.srvCount = 0u;

    for ( int i = 0; i < 16; i++ ) {
        const auto& attachment = description.attachements[i];

        switch ( attachment.bindMode ) {
        case RenderPassDesc::READ:
        {
            Texture* texture = nullptr;
            
            if ( attachment.targetState == RenderPassDesc::IS_TEXTURE )
                texture = attachment.texture;
            else if ( attachment.targetState == RenderPassDesc::IS_UAV_TEXTURE )
                texture = attachment.buffer->bufferTexture;
            else
                texture = attachment.renderTarget->texture;

            if ( attachment.stageBind & eShaderStage::SHADER_STAGE_PIXEL )
                renderPass->pixelStage.shaderResourceView[renderPass->pixelStage.srvCount++] = ( texture == nullptr ) ? nullptr : texture->shaderResourceView;
            if ( attachment.stageBind & eShaderStage::SHADER_STAGE_COMPUTE )
                renderPass->computeStage.shaderResourceView[renderPass->computeStage.srvCount++] = ( texture == nullptr ) ? nullptr : texture->shaderResourceView;
        } break;

        case RenderPassDesc::WRITE:
            renderPass->clearTarget[renderPass->rtvCount] = ( attachment.targetState != RenderPassDesc::DONT_CARE );
            memcpy( renderPass->clearValue[renderPass->rtvCount], attachment.clearValue, sizeof( FLOAT ) * 4 );

            if ( attachment.layerIndex != 0 )
                renderPass->renderTargetViews[renderPass->rtvCount++] = ( attachment.mipIndex != 0 ) 
                    ? attachment.renderTarget->textureRenderTargetViewPerSliceAndMipLevel[attachment.layerIndex][attachment.mipIndex]
                    : attachment.renderTarget->textureRenderTargetViewPerSlice[attachment.layerIndex];
            else
                renderPass->renderTargetViews[renderPass->rtvCount++] = ( attachment.mipIndex != 0 )
                    ? attachment.renderTarget->textureRenderTargetViewPerSliceAndMipLevel[0][attachment.mipIndex]
                    : attachment.renderTarget->textureRenderTargetView;
            break;

        case RenderPassDesc::WRITE_DEPTH:
            renderPass->clearTarget[8] = ( attachment.targetState != RenderPassDesc::DONT_CARE );
            memcpy( renderPass->clearValue[8], attachment.clearValue, sizeof( FLOAT ) * 4 );

            renderPass->depthStencilView = attachment.renderTarget->textureDepthRenderTargetView;
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
            CommandListObject->deferredContext->ClearRenderTargetView( renderPass->renderTargetViews[i], renderPass->clearValue[i] );
        }
    }
    
    if ( renderPass->clearTarget[8] ) {
        CommandListObject->deferredContext->ClearDepthStencilView( renderPass->depthStencilView, D3D11_CLEAR_DEPTH, renderPass->clearValue[8][0], 0x0 );
    }
    
    // TODO Avoid resource binding dependencies (skip explicit SRV unbind)?
    static constexpr ID3D11ShaderResourceView* NO_SRV[8] = { ( ID3D11ShaderResourceView* )nullptr };

    CommandListObject->deferredContext->PSSetShaderResources( 0, 8, NO_SRV );
    CommandListObject->deferredContext->CSSetShaderResources( 0, 8, NO_SRV );

    CommandListObject->deferredContext->OMSetRenderTargets( renderPass->rtvCount, renderPass->renderTargetViews, renderPass->depthStencilView );

    CommandListObject->deferredContext->PSSetShaderResources( 0, 8, renderPass->pixelStage.shaderResourceView );
    CommandListObject->deferredContext->CSSetShaderResources( 0, 8, renderPass->computeStage.shaderResourceView );


}
#endif
