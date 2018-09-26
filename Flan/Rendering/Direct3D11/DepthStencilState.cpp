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
#include "DepthStencilState.h"

#include <Rendering/StencilOperation.h>

#include "RenderContext.h"
#include "CommandList.h"
#include "ComparisonFunctions.h"

using namespace flan::rendering;

static constexpr D3D11_STENCIL_OP D3D111_STENCIL_OPERATION[eStencilOperation::StencilOperation_COUNT] =
{
    D3D11_STENCIL_OP_KEEP,
    D3D11_STENCIL_OP_ZERO,
    D3D11_STENCIL_OP_REPLACE,
    D3D11_STENCIL_OP_INCR,
    D3D11_STENCIL_OP_INCR_SAT,
    D3D11_STENCIL_OP_DECR,
    D3D11_STENCIL_OP_DECR_SAT,
    D3D11_STENCIL_OP_INVERT
};

NativeDepthStencilStateObject* flan::rendering::CreateDepthStencilStateImpl( NativeRenderContext* nativeRenderContext, const DepthStencilStateDesc& description )
{
    const D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {
        static_cast<BOOL>( description.enableDepthTest ),
        description.enableDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO,
        D3D11_COMPARISON_FUNCTION[description.depthComparisonFunc],
        static_cast<BOOL>( description.enableStencilTest ),
        description.stencilReadMask,							// UINT8 StencilReadMask
        description.stencilWriteMask,							// UINT8 StencilWriteMask

        // D3D11_DEPTH_STENCILOP_DESC FrontFace
        {								
            D3D111_STENCIL_OPERATION[description.front.failOp],		        //		D3D11_STENCIL_OP StencilFailOp
            D3D111_STENCIL_OPERATION[description.front.zFailOp],		    //		D3D11_STENCIL_OP StencilDepthFailOp
            D3D111_STENCIL_OPERATION[description.front.passOp],		        //		D3D11_STENCIL_OP StencilPassOp
            D3D11_COMPARISON_FUNCTION[description.front.comparisonFunc],	//		D3D11_COMPARISON_FUNC StencilFunc
        },

        // D3D11_DEPTH_STENCILOP_DESC BackFace
        {
            D3D111_STENCIL_OPERATION[description.back.failOp],		        //		D3D11_STENCIL_OP StencilFailOp
            D3D111_STENCIL_OPERATION[description.back.zFailOp],		    //		D3D11_STENCIL_OP StencilDepthFailOp
            D3D111_STENCIL_OPERATION[description.back.passOp],		        //		D3D11_STENCIL_OP StencilPassOp
            D3D11_COMPARISON_FUNCTION[description.back.comparisonFunc],	//		D3D11_COMPARISON_FUNC StencilFunc
        },
    };

    NativeDepthStencilStateObject* depthStencilState = new NativeDepthStencilStateObject();
    nativeRenderContext->nativeDevice->CreateDepthStencilState( &depthStencilDesc, &depthStencilState->depthStencilStateObject );
    depthStencilState->stencilRef = description.stencilRefValue;

    return depthStencilState;
}

void flan::rendering::DestroyDepthStencilStateImpl( NativeRenderContext* nativeRenderContext, NativeDepthStencilStateObject* depthStencilStateObject )
{
#define D3D11_RELEASE( obj ) if ( obj != nullptr ) { obj->Release(); obj = nullptr; }
    D3D11_RELEASE( depthStencilStateObject->depthStencilStateObject );
}

void flan::rendering::BindDepthStencilStateCmdImpl( NativeCommandList* nativeCommandList, NativeDepthStencilStateObject* depthStencilStateObject )
{
    nativeCommandList->deferredContext->OMSetDepthStencilState( depthStencilStateObject->depthStencilStateObject, depthStencilStateObject->stencilRef );
}
#endif
