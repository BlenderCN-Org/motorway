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
#pragma once

#if FLAN_D3D11
#include <Rendering/PipelineState.h>

#include <d3d11.h>

struct NativeRenderContext;

struct NativePipelineStateObject
{
    ID3D11InputLayout* inputLayout;
    D3D11_PRIMITIVE_TOPOLOGY primitiveTopology;

    ID3D11VertexShader*     vertexStage;
    ID3D11HullShader*       tesselationControlStage;
    ID3D11DomainShader*     tesselationEvalStage;
    ID3D11PixelShader*      pixelStage;
    ID3D11ComputeShader*    computeStage;

    NativePipelineStateObject()
        : inputLayout( nullptr )
        , vertexStage( nullptr )
        , tesselationControlStage( nullptr )
        , tesselationEvalStage( nullptr )
        , pixelStage( nullptr )
        , computeStage( nullptr )
    {
    
    }
};

namespace flan
{
    namespace rendering
    {
        NativePipelineStateObject* CreatePipelineStateImpl( NativeRenderContext* nativeRenderContext, const PipelineStateDesc& description );
        void DestroyPipelineStateImpl( NativeRenderContext* nativeRenderContext, NativePipelineStateObject* PipelineStateObject );
    }
}
#endif
