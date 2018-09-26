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
#include "PipelineState.h"

#include <d3d11.h>

#include "RenderContext.h"
#include "Shader.h"

#include "PrimtiveTopologies.h"

ID3D11InputLayout* CreateInputLayout( ID3D11Device* nativeDevice, const InputLayout_t& inputLayout, const std::vector<uint8_t>* shaderBytecode )
{
    D3D11_INPUT_ELEMENT_DESC* layoutDescription = new D3D11_INPUT_ELEMENT_DESC[inputLayout.size()]{};

    for ( unsigned int i = 0; i < inputLayout.size(); i++ ) {
        layoutDescription[i] = {
            inputLayout[i].semanticName,
            inputLayout[i].index,
            static_cast< DXGI_FORMAT >( inputLayout[i].format ),
            inputLayout[i].vertexBufferIndex,
            inputLayout[i].needPadding ? D3D11_APPEND_ALIGNED_ELEMENT : inputLayout[i].offsetInBytes,
            inputLayout[i].instanceCount == 0 ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA,
            inputLayout[i].instanceCount
        };
    }

    ID3D11InputLayout* inputLayoutObject = nullptr;
    nativeDevice->CreateInputLayout( layoutDescription, ( UINT )inputLayout.size(), shaderBytecode->data(), ( UINT )shaderBytecode->size(), &inputLayoutObject );

    delete[] layoutDescription;

    return inputLayoutObject;
}

NativePipelineStateObject* flan::rendering::CreatePipelineStateImpl( NativeRenderContext* nativeRenderContext, const PipelineStateDesc& description )
{
    auto inputLayout = description.inputLayout;

    NativePipelineStateObject* nativePipelineStateObject = new NativePipelineStateObject();
    nativePipelineStateObject->primitiveTopology = _D3D11_PRIMITIVE_TOPOLOGY[description.primitiveTopology];
    
    if ( !description.inputLayout.empty() ) {
        nativePipelineStateObject->inputLayout = CreateInputLayout( nativeRenderContext->nativeDevice, description.inputLayout, description.vertexStage->getShaderBytecode() );
    }

#define D3D11_ADD_STAGE_TO_PIPELINE( stageName )\
    if ( description.stageName##Stage != nullptr ) {\
        auto stageName##StageNative = description.stageName##Stage->getNativeShaderObject();\
        nativePipelineStateObject->stageName##Stage = stageName##StageNative->stageName##Shader;\
    }

    D3D11_ADD_STAGE_TO_PIPELINE( vertex );
    D3D11_ADD_STAGE_TO_PIPELINE( tesselationControl );
    D3D11_ADD_STAGE_TO_PIPELINE( tesselationEval );
    D3D11_ADD_STAGE_TO_PIPELINE( pixel );
    D3D11_ADD_STAGE_TO_PIPELINE( compute );
#undef D3D11_ADD_STAGE_TO_PIPELINE

    return nativePipelineStateObject;
}

void flan::rendering::DestroyPipelineStateImpl( NativeRenderContext* nativeRenderContext, NativePipelineStateObject* PipelineStateObject )
{
#define D3D11_RELEASE( obj ) if ( obj != nullptr ) { obj->Release(); obj = nullptr; }
    D3D11_RELEASE( PipelineStateObject->inputLayout );
}
#endif
