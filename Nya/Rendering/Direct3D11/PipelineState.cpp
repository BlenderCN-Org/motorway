/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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
#include "Shader.h"
#include "ComparisonFunctions.h"
#include "PipelineState.h"
#include <d3d11.h>

using namespace nya::rendering;

static constexpr D3D11_PRIMITIVE_TOPOLOGY _D3D11_PRIMITIVE_TOPOLOGY[ePrimitiveTopology::PRIMITIVE_TOPOLOGY_COUNT] = 
{
    D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
    D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
    D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST
};

static constexpr D3D11_BLEND D3D11_BLEND_SOURCE[eBlendSource::BLEND_SOURCE_COUNT] = 
{
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

static constexpr D3D11_BLEND_OP D3D11_BLEND_OPERATION[eBlendOperation::BLEND_OPERATION_COUNT] = 
{
    D3D11_BLEND_OP::D3D11_BLEND_OP_ADD,
    D3D11_BLEND_OP::D3D11_BLEND_OP_SUBTRACT,
    D3D11_BLEND_OP::D3D11_BLEND_OP_MIN,
    D3D11_BLEND_OP::D3D11_BLEND_OP_MAX,
};

static constexpr D3D11_STENCIL_OP D3D111_STENCIL_OPERATION[eStencilOperation::STENCIL_OPERATION_COUNT] =
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

static constexpr D3D11_FILL_MODE D3D11_FM[eFillMode::FILL_MODE_COUNT] = 
{
    D3D11_FILL_MODE::D3D11_FILL_SOLID,
    D3D11_FILL_MODE::D3D11_FILL_WIREFRAME,
};

static constexpr D3D11_CULL_MODE D3D11_CM[eCullMode::CULL_MODE_COUNT] = 
{
    D3D11_CULL_MODE::D3D11_CULL_NONE,
    D3D11_CULL_MODE::D3D11_CULL_FRONT,
    D3D11_CULL_MODE::D3D11_CULL_BACK,
    D3D11_CULL_MODE::D3D11_CULL_BACK,
};

PipelineState::ResourceListLayout::ResourceBindType GetBindType(const ResourceListLayoutDesc::ResourceType resourceType, const bool readOnlyUAV = false )
{
    switch ( resourceType ) {
    case ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER:
        return PipelineState::ResourceListLayout::ResourceBindType::Sampler;
    case ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_TEXTURE:
        return PipelineState::ResourceListLayout::ResourceBindType::Texture;
    case ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_RENDER_TARGET:
        return PipelineState::ResourceListLayout::ResourceBindType::RenderTarget; 
    case ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_UAV_BUFFER:
        return ( readOnlyUAV ) ? PipelineState::ResourceListLayout::ResourceBindType::Buffer : PipelineState::ResourceListLayout::ResourceBindType::UAVResource;
    case ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_UAV_TEXTURE:
        return (readOnlyUAV) ? PipelineState::ResourceListLayout::ResourceBindType::Texture : PipelineState::ResourceListLayout::ResourceBindType::UAVResource;
    case ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER:
        return PipelineState::ResourceListLayout::ResourceBindType::CBuffer;
    default:
        return PipelineState::ResourceListLayout::ResourceBindType::Buffer;
    }
}

ID3D11InputLayout* CreateInputLayout( ID3D11Device* nativeDevice, const InputLayoutEntry* inputLayout, const void* shaderBytecode, const size_t shaderBytecodeSize )
{
    D3D11_INPUT_ELEMENT_DESC layoutDescription[8];

    unsigned int i = 0;
    for ( ; i < 8; i++ ) {
        if ( inputLayout[i].semanticName == nullptr ) {
            break;
        }

        layoutDescription[i] = {
            inputLayout[i].semanticName,
            inputLayout[i].index,
            static_cast< DXGI_FORMAT >( inputLayout[i].format ),
            inputLayout[i].vertexBufferIndex,
            ( inputLayout[i].needPadding ) ? D3D11_APPEND_ALIGNED_ELEMENT : inputLayout[i].offsetInBytes,
            ( inputLayout[i].instanceCount == 0 ) ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA,
            inputLayout[i].instanceCount
        };
    }

    if ( i == 0 ) {
        return nullptr;
    }

    ID3D11InputLayout* inputLayoutObject = nullptr;
    nativeDevice->CreateInputLayout( layoutDescription, i, shaderBytecode, ( UINT )shaderBytecodeSize, &inputLayoutObject );

    return inputLayoutObject;
}

ID3D11BlendState* CreateBlendState( ID3D11Device* nativeDevice, const BlendStateDesc& description )
{
    if ( !description.enableBlend ) {
        return nullptr;
    }

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
    
    ID3D11BlendState* blendState;
    nativeDevice->CreateBlendState( &blendDesc, &blendState );   
    return blendState;
}

ID3D11DepthStencilState* CreateDepthStencilState( ID3D11Device* nativeDevice, const DepthStencilStateDesc& description )
{
    const D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {
        static_cast<BOOL>( description.enableDepthTest ),
        ( description.enableDepthWrite ) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO,
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

    ID3D11DepthStencilState* depthStencilState;
    nativeDevice->CreateDepthStencilState( &depthStencilDesc, &depthStencilState );

    return depthStencilState;
}

ID3D11RasterizerState* CreateRasterizerState( ID3D11Device* nativeDevice, const RasterizerStateDesc& description )
{
    const D3D11_RASTERIZER_DESC rasterDesc =
    {
        D3D11_FM[description.fillMode],
        D3D11_CM[description.cullMode],
        static_cast<BOOL>( description.useTriangleCCW ),
        ( INT )description.depthBias,          // INT DepthBias;
        description.depthBiasClamp,       // FLOAT DepthBiasClamp;
        description.slopeScale,       // FLOAT SlopeScaledDepthBias;
        TRUE,       // BOOL DepthClipEnable;
        0,          // BOOL ScissorEnable;
        0,          // BOOL MultisampleEnable;
        0,          // BOOL AntialiasedLineEnable;
    };

    ID3D11RasterizerState* rasterizerState;
    nativeDevice->CreateRasterizerState( &rasterDesc, &rasterizerState );

    return rasterizerState;
}

PipelineState* RenderDevice::createPipelineState( const PipelineStateDesc& description )
{
    PipelineState* pipelineState = nya::core::allocate<PipelineState>( memoryAllocator );

#define BIND_IF_AVAILABLE( stage ) pipelineState->stage = ( description.stage != nullptr ) ? description.stage->stage : nullptr;
    BIND_IF_AVAILABLE( vertexShader )
    BIND_IF_AVAILABLE( tesselationControlShader )
    BIND_IF_AVAILABLE( tesselationEvalShader )
    BIND_IF_AVAILABLE( pixelShader )
    BIND_IF_AVAILABLE( computeShader )
#undef BIND_IF_AVAILABLE

    if ( description.vertexShader != nullptr ) {
        pipelineState->inputLayout = CreateInputLayout( renderContext->nativeDevice, description.inputLayout, description.vertexShader->bytecode, description.vertexShader->bytecodeSize );
    }

    pipelineState->primitiveTopology = _D3D11_PRIMITIVE_TOPOLOGY[description.primitiveTopology];

    pipelineState->blendState = CreateBlendState( renderContext->nativeDevice, description.blendState );
    pipelineState->blendMask = static_cast<UINT>( description.blendState.sampleMask );

    pipelineState->depthStencilState = CreateDepthStencilState( renderContext->nativeDevice, description.depthStencilState );
    pipelineState->stencilRef = static_cast<UINT>( description.depthStencilState.stencilRefValue );

    pipelineState->rasterizerState = CreateRasterizerState( renderContext->nativeDevice, description.rasterizerState );

    pipelineState->resourceList.resourceToBindCount = 0u;

    int32_t& resourceBindCount = pipelineState->resourceList.resourceToBindCount;
    for ( int i = 0; i < MAX_RES_COUNT; i++ ) {
        const auto& resource = description.resourceListLayout.resources[i];
        
        if ( resource.type == ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER ) {
            if ( resource.stageBind & SHADER_STAGE_VERTEX ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].samplerState = &pipelineState->resourceList.samplers.vertexStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_TESSELATION_CONTROL ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].samplerState = &pipelineState->resourceList.samplers.hullStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_TESSELATION_EVALUATION ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].samplerState = &pipelineState->resourceList.samplers.domainStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_PIXEL ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].samplerState = &pipelineState->resourceList.samplers.pixelStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_COMPUTE ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].samplerState = &pipelineState->resourceList.samplers.computeStage[resource.bindPoint];
            }
        } else if ( resource.type == ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER ) {
            if ( resource.stageBind & SHADER_STAGE_VERTEX ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].buffers = &pipelineState->resourceList.constantBuffers.vertexStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_TESSELATION_CONTROL ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].buffers = &pipelineState->resourceList.constantBuffers.hullStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_TESSELATION_EVALUATION ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].buffers = &pipelineState->resourceList.constantBuffers.domainStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_PIXEL ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].buffers = &pipelineState->resourceList.constantBuffers.pixelStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_COMPUTE ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].buffers = &pipelineState->resourceList.constantBuffers.computeStage[resource.bindPoint];
            }
        } else if ( resource.type == ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_UAV_BUFFER
                 || resource.type == ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_UAV_TEXTURE ) {
            if ( resource.stageBind & SHADER_STAGE_VERTEX ) {
               pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
               pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type, true);
               pipelineState->resourceList.resources[resourceBindCount++].shaderResourceView = &pipelineState->resourceList.shaderResourceViews.vertexStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_TESSELATION_CONTROL ) {
               pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
               pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type, true);
               pipelineState->resourceList.resources[resourceBindCount++].shaderResourceView = &pipelineState->resourceList.shaderResourceViews.hullStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_TESSELATION_EVALUATION ) {
               pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
               pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type, true);
               pipelineState->resourceList.resources[resourceBindCount++].shaderResourceView = &pipelineState->resourceList.shaderResourceViews.domainStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_PIXEL ) {
               pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
               pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type, true);
               pipelineState->resourceList.resources[resourceBindCount++].shaderResourceView = &pipelineState->resourceList.shaderResourceViews.pixelStage[resource.bindPoint];
            }

            if ( resource.stageBind & SHADER_STAGE_COMPUTE ) {
               pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
               pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type, false);
               pipelineState->resourceList.resources[resourceBindCount++].unorderedAccessView = &pipelineState->resourceList.uavBuffers[resource.bindPoint];
               pipelineState->resourceList.uavBuffersBindCount++;
            }
        } else {
            uint32_t resourceRegisterIdxShift = ( resource.type == ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_RENDER_TARGET
                                               || resource.type == ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_TEXTURE ) ? 0u : 0u;
            if ( resource.stageBind & SHADER_STAGE_VERTEX ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].shaderResourceView = &pipelineState->resourceList.shaderResourceViews.vertexStage[resource.bindPoint - resourceRegisterIdxShift];
            }

            if ( resource.stageBind & SHADER_STAGE_TESSELATION_CONTROL ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].shaderResourceView = &pipelineState->resourceList.shaderResourceViews.hullStage[resource.bindPoint - resourceRegisterIdxShift];
            }

            if ( resource.stageBind & SHADER_STAGE_TESSELATION_EVALUATION ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].shaderResourceView = &pipelineState->resourceList.shaderResourceViews.domainStage[resource.bindPoint - resourceRegisterIdxShift];
            }

            if ( resource.stageBind & SHADER_STAGE_PIXEL ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].shaderResourceView = &pipelineState->resourceList.shaderResourceViews.pixelStage[resource.bindPoint - resourceRegisterIdxShift];
            }

            if ( resource.stageBind & SHADER_STAGE_COMPUTE ) {
                pipelineState->resourceList.resources[resourceBindCount].resourceIndex = i;
                pipelineState->resourceList.resources[resourceBindCount].type = GetBindType( resource.type );
                pipelineState->resourceList.resources[resourceBindCount++].shaderResourceView = &pipelineState->resourceList.shaderResourceViews.computeStage[resource.bindPoint - resourceRegisterIdxShift];
            }
        }
    }

    pipelineState->renderPass.pixelStage.srvCount = 0u;
    pipelineState->renderPass.computeStage.srvCount = 0u;
    pipelineState->renderPass.resourceToBindCount = 0;
    pipelineState->renderPass.rtvCount = 0u;

    auto& renderPass = pipelineState->renderPass; 
    renderPass = { 0 };

    for ( int i = 0; i < 24; i++ ) {
        const auto& attachment = description.renderPassLayout.attachements[i];

        if ( attachment.bindMode == RenderPassLayoutDesc::UNUSED ) {
            break;
        }

        switch ( attachment.bindMode ) {
        case RenderPassLayoutDesc::READ:
        {
            if ( attachment.stageBind & eShaderStage::SHADER_STAGE_PIXEL ) {
                renderPass.resources[renderPass.resourceToBindCount] = { i, PipelineState::RenderPassLayout::ShaderResourceView, nullptr };
                renderPass.resources[renderPass.resourceToBindCount++].shaderResourceView = &renderPass.pixelStage.shaderResourceView[renderPass.pixelStage.srvCount++];
            }

            if ( attachment.stageBind & eShaderStage::SHADER_STAGE_COMPUTE ) {
                renderPass.resources[renderPass.resourceToBindCount] = { i, PipelineState::RenderPassLayout::ShaderResourceView, nullptr };
                renderPass.resources[renderPass.resourceToBindCount++].shaderResourceView = &renderPass.computeStage.shaderResourceView[renderPass.computeStage.srvCount++];
            }
        } break;

        case RenderPassLayoutDesc::SWAPCHAIN_BUFFER:
        case RenderPassLayoutDesc::WRITE:
            renderPass.clearTarget[renderPass.rtvCount] = ( attachment.targetState != RenderPassLayoutDesc::DONT_CARE );
            memcpy( renderPass.clearValue[renderPass.rtvCount], attachment.clearValue, sizeof( FLOAT ) * 4 );

            renderPass.resources[renderPass.resourceToBindCount] = { i, PipelineState::RenderPassLayout::RenderTargetView, nullptr };
            renderPass.resources[renderPass.resourceToBindCount++].renderTargetView = &renderPass.renderTargetViews[renderPass.rtvCount++];
            break;

        case RenderPassLayoutDesc::WRITE_DEPTH:
            renderPass.clearTarget[8] = ( attachment.targetState != RenderPassLayoutDesc::DONT_CARE );
            memcpy( renderPass.clearValue[8], attachment.clearValue, sizeof( FLOAT ) * 4 );

            renderPass.resources[renderPass.resourceToBindCount] = { i, PipelineState::RenderPassLayout::DepthStencilView, nullptr };
            renderPass.resources[renderPass.resourceToBindCount++].depthStencilView = &renderPass.depthStencilView;
            break;

        default:
            break;
        }
    }
    return pipelineState;
}

void RenderDevice::destroyPipelineState( PipelineState* pipelineState )
{
#define D3D11_RELEASE( obj ) if ( obj != nullptr ) { obj->Release(); obj = nullptr; }

    D3D11_RELEASE( pipelineState->rasterizerState );
    D3D11_RELEASE( pipelineState->depthStencilState );
    D3D11_RELEASE( pipelineState->blendState );
    D3D11_RELEASE( pipelineState->inputLayout );

    nya::core::free( memoryAllocator, pipelineState );
}

void CommandList::bindPipelineState( PipelineState* pipelineState )
{
    constexpr FLOAT BLEND_FACTORS[4] = { 1.0F, 1.0F, 1.0F, 1.0F };

    ID3D11DeviceContext* deviceContext = CommandListObject->deferredContext;

    deviceContext->IASetPrimitiveTopology( pipelineState->primitiveTopology );
    deviceContext->IASetInputLayout( pipelineState->inputLayout );

    deviceContext->OMSetDepthStencilState( pipelineState->depthStencilState, pipelineState->stencilRef );
    deviceContext->OMSetBlendState( pipelineState->blendState, BLEND_FACTORS, pipelineState->blendMask );

    deviceContext->RSSetState( pipelineState->rasterizerState );

    deviceContext->VSSetShader( pipelineState->vertexShader, nullptr, 0 );
    deviceContext->HSSetShader( pipelineState->tesselationControlShader, nullptr, 0 );
    deviceContext->DSSetShader( pipelineState->tesselationEvalShader, nullptr, 0 );
    deviceContext->PSSetShader( pipelineState->pixelShader, nullptr, 0 );
    deviceContext->CSSetShader( pipelineState->computeShader, nullptr, 0 );

    auto& resourceListLayout = pipelineState->resourceList;

    // For extra-safety, unbind SRV resources prior to UAV bindings
    if ( resourceListLayout.uavBuffersBindCount != 0 ) {
        static constexpr ID3D11ShaderResourceView* NO_SRV[20] = { ( ID3D11ShaderResourceView* )nullptr };
        static constexpr ID3D11RenderTargetView* NO_RTV[8] = { ( ID3D11RenderTargetView* )nullptr };
        CommandListObject->deferredContext->OMSetRenderTargets( 8u, NO_RTV, nullptr );

        CommandListObject->deferredContext->VSSetShaderResources( 0, 20, NO_SRV );
        CommandListObject->deferredContext->HSSetShaderResources( 0, 20, NO_SRV );
        CommandListObject->deferredContext->DSSetShaderResources( 0, 20, NO_SRV );
        CommandListObject->deferredContext->PSSetShaderResources( 0, 20, NO_SRV );
        CommandListObject->deferredContext->CSSetShaderResources( 0, 20, NO_SRV );
    }

    CommandListObject->deferredContext->CSSetUnorderedAccessViews( 0, 7, resourceListLayout.uavBuffers, nullptr );

    CommandListObject->deferredContext->VSSetSamplers( 0, 16, resourceListLayout.samplers.vertexStage );
    CommandListObject->deferredContext->HSSetSamplers( 0, 16, resourceListLayout.samplers.hullStage );
    CommandListObject->deferredContext->DSSetSamplers( 0, 16, resourceListLayout.samplers.domainStage );
    CommandListObject->deferredContext->PSSetSamplers( 0, 16, resourceListLayout.samplers.pixelStage );
    CommandListObject->deferredContext->CSSetSamplers( 0, 16, resourceListLayout.samplers.computeStage );

    CommandListObject->deferredContext->VSSetConstantBuffers( 0, 14, resourceListLayout.constantBuffers.vertexStage );
    CommandListObject->deferredContext->HSSetConstantBuffers( 0, 14, resourceListLayout.constantBuffers.hullStage );
    CommandListObject->deferredContext->DSSetConstantBuffers( 0, 14, resourceListLayout.constantBuffers.domainStage );
    CommandListObject->deferredContext->PSSetConstantBuffers( 0, 14, resourceListLayout.constantBuffers.pixelStage );
    CommandListObject->deferredContext->CSSetConstantBuffers( 0, 14, resourceListLayout.constantBuffers.computeStage );

    CommandListObject->deferredContext->VSSetShaderResources( 0, 20, resourceListLayout.shaderResourceViews.vertexStage );
    CommandListObject->deferredContext->HSSetShaderResources( 0, 20, resourceListLayout.shaderResourceViews.hullStage );
    CommandListObject->deferredContext->DSSetShaderResources( 0, 20, resourceListLayout.shaderResourceViews.domainStage );
    CommandListObject->deferredContext->PSSetShaderResources( 0, 20, resourceListLayout.shaderResourceViews.pixelStage );
    CommandListObject->deferredContext->CSSetShaderResources( 0, 20, resourceListLayout.shaderResourceViews.computeStage );
}
#endif
