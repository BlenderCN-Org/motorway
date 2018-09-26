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

#if FLAN_D3D11
#include "BlendState.h"

#include "RenderContext.h"
#include "CommandList.h"

#include <Rendering/BlendOperations.h>
#include <Rendering/BlendSources.h>

using namespace flan::rendering;

static constexpr D3D11_BLEND D3D11_BLEND_SOURCE[eBlendSource::BlendSource_COUNT] = {
    D3D11_BLEND::D3D11_BLEND_ZERO,
    D3D11_BLEND::D3D11_BLEND_ONE,

    D3D11_BLEND::D3D11_BLEND_SRC_COLOR,
    D3D11_BLEND::D3D11_BLEND_INV_SRC_COLOR,

    D3D11_BLEND::D3D11_BLEND_SRC_ALPHA,
    D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA,

    D3D11_BLEND::D3D11_BLEND_DEST_ALPHA,
    D3D11_BLEND::D3D11_BLEND_INV_DEST_ALPHA,

    D3D11_BLEND::D3D11_BLEND_DEST_COLOR,
    D3D11_BLEND::D3D11_BLEND_INV_DEST_COLOR,

    D3D11_BLEND::D3D11_BLEND_SRC_ALPHA_SAT,

    D3D11_BLEND::D3D11_BLEND_BLEND_FACTOR,
    D3D11_BLEND::D3D11_BLEND_INV_BLEND_FACTOR,
};

static constexpr D3D11_BLEND_OP D3D11_BLEND_OPERATION[eBlendOperation::BlendOperation_COUNT] = {
    D3D11_BLEND_OP::D3D11_BLEND_OP_ADD,
    D3D11_BLEND_OP::D3D11_BLEND_OP_SUBTRACT,
    D3D11_BLEND_OP::D3D11_BLEND_OP_MIN,
    D3D11_BLEND_OP::D3D11_BLEND_OP_MAX,
};

NativeBlendStateObject* flan::rendering::CreateBlendStateImpl( NativeRenderContext* nativeRenderContext, const BlendStateDesc& description )
{
    NativeBlendStateObject* blendState = new NativeBlendStateObject();
    blendState->sampleMask = description.sampleMask;

    D3D11_BLEND_DESC blendDesc = { 0 };
    blendDesc.AlphaToCoverageEnable = description.enableAlphaToCoverage;
    blendDesc.IndependentBlendEnable = FALSE;

    for ( int i = 0; i < 8; i++ ) {
        blendDesc.RenderTarget[i].BlendEnable = description.enableBlend;
        blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SOURCE[description.blendConfColor.source];
        blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_SOURCE[description.blendConfColor.dest];
        blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OPERATION[description.blendConfColor.operation];
        blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_SOURCE[description.blendConfAlpha.source];
        blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_SOURCE[description.blendConfAlpha.dest];
        blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OPERATION[description.blendConfAlpha.operation];
        blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }

    nativeRenderContext->nativeDevice->CreateBlendState( &blendDesc, &blendState->blendState );

    return blendState;
}

void flan::rendering::DestroyBlendStateImpl( NativeRenderContext* nativeRenderContext, NativeBlendStateObject* blendStateObject )
{
    blendStateObject->blendState->Release();
    blendStateObject->sampleMask = 0;
}

void flan::rendering::BindBlendStateCmdImpl( NativeCommandList* nativeCmdList, NativeBlendStateObject* blendStateObject )
{
    constexpr FLOAT BLEND_FACTORS[4] = { 1.0F, 1.0F, 1.0F, 1.0F };

    nativeCmdList->deferredContext->OMSetBlendState( blendStateObject->blendState, BLEND_FACTORS, blendStateObject->sampleMask );
}
#endif
