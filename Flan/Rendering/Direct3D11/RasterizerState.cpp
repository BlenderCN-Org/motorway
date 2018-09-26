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
#include "RasterizerState.h"
#include "RenderContext.h"
#include "CommandList.h"

#include <Rendering/FillModes.h>
#include <Rendering/CullModes.h>

using namespace flan::rendering;

static constexpr D3D11_FILL_MODE D3D11_FM[FillMode_COUNT] = {
    D3D11_FILL_MODE::D3D11_FILL_SOLID,
    D3D11_FILL_MODE::D3D11_FILL_WIREFRAME,
};

static constexpr D3D11_CULL_MODE D3D11_CM[CullMode_COUNT] = {
    D3D11_CULL_MODE::D3D11_CULL_NONE,
    D3D11_CULL_MODE::D3D11_CULL_FRONT,
    D3D11_CULL_MODE::D3D11_CULL_BACK,
    D3D11_CULL_MODE::D3D11_CULL_BACK,
};

NativeRasterizerStateObject* flan::rendering::CreateRasterizerStateImpl( NativeRenderContext* nativeRenderContext, const RasterizerStateDesc& description )
{
    const D3D11_RASTERIZER_DESC rasterDesc =
    {
        D3D11_FM[description.fillMode],
        D3D11_CM[description.cullMode],
        static_cast<BOOL>( description.useTriangleCCW ),
        (INT)description.depthBias,          // INT DepthBias;
        description.depthBiasClamp,       // FLOAT DepthBiasClamp;
        description.slopeScale,       // FLOAT SlopeScaledDepthBias;
        TRUE,       // BOOL DepthClipEnable;
        0,          // BOOL ScissorEnable;
        0,          // BOOL MultisampleEnable;
        0,          // BOOL AntialiasedLineEnable;
    };

    NativeRasterizerStateObject* rasterizerState = new NativeRasterizerStateObject();
    nativeRenderContext->nativeDevice->CreateRasterizerState( &rasterDesc, &rasterizerState->rasterizerStateObject );

    return rasterizerState;
}

void flan::rendering::DestroyRasterizerStateImpl( NativeRenderContext* nativeRenderContext, NativeRasterizerStateObject* rasterizerStateObject )
{
#define D3D11_RELEASE( obj ) if ( obj != nullptr ) { obj->Release(); obj = nullptr; }
    D3D11_RELEASE( rasterizerStateObject->rasterizerStateObject );
}

void flan::rendering::BindRasterizerStateCmdImpl( NativeCommandList* nativeCmdList, NativeRasterizerStateObject* rasterizerStateObject )
{
    nativeCmdList->deferredContext->RSSetState( rasterizerStateObject->rasterizerStateObject );
}
#endif
