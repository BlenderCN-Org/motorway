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
#include "CommandList.h"
#include "RenderContext.h"

#include "PipelineState.h"
#include "RenderTarget.h"

#include <d3d11.h>

NativeCommandList* flan::rendering::CreateCommandListImpl( NativeRenderContext* renderContext )
{
    HRESULT operationResult = S_OK;
    ID3D11DeviceContext* deferredContext = nullptr;

    operationResult = renderContext->nativeDevice->CreateDeferredContext( 0, &deferredContext );

    if ( FAILED( operationResult ) ) {
        FLAN_CERR << "Failed to create deferred context (error code: 0x" << std::hex << operationResult << std::dec << ")" << std::endl;
        return nullptr;
    }

    NativeCommandList* cmdList = new NativeCommandList();
    cmdList->deferredContext = deferredContext;
    cmdList->cmdList = nullptr;
    cmdList->backbuffer = renderContext->backbuffer;

    return cmdList;
}

void flan::rendering::DestroyCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{
#define D3D11_RELEASE( object ) if ( object != nullptr ) { object->Release(); object = nullptr; }
    D3D11_RELEASE( cmdList->deferredContext );
#undef D3D11_RELEASE
}

void flan::rendering::BeginCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{

}

void flan::rendering::EndCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{
    HRESULT operationResult = S_OK;
    ID3D11CommandList* nativeCmdList = nullptr;

    operationResult = cmdList->deferredContext->FinishCommandList( FALSE, &nativeCmdList );

    if ( FAILED( operationResult ) ) {
        FLAN_CERR << "Failed to create command list (error code: 0x" << std::hex << operationResult << std::dec << ")" << std::endl;
        return;
    }

    cmdList->cmdList = nativeCmdList;
}

void flan::rendering::PlayBackCommandListImpl( NativeRenderContext* renderContext, NativeCommandList* cmdList )
{
    renderContext->nativeDeviceContext->ExecuteCommandList( cmdList->cmdList, FALSE );

    // Release CmdList (afaik you can't recycle them in d3d11)
    cmdList->cmdList->Release();
}

void flan::rendering::SetDepthClearValue( NativeCommandList* cmdList, const float depthClearValue )
{
    cmdList->depthClearValue = depthClearValue;
}

void flan::rendering::ClearRenderTargetImpl( NativeCommandList* cmdList )
{
    ClearColorRenderTargetImpl( cmdList );
    ClearDepthRenderTargetImpl( cmdList );
}

void flan::rendering::ClearColorRenderTargetImpl( NativeCommandList* cmdList )
{
    static constexpr FLOAT CLEAR_COLOR[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    auto nativeDeviceContext = cmdList->deferredContext;
    for ( int targetId = 0; targetId < cmdList->currentRenderTargetCount; targetId++ ) {
        nativeDeviceContext->ClearRenderTargetView( cmdList->currentRenderTargets[targetId], CLEAR_COLOR );
    }
}

void flan::rendering::ClearDepthRenderTargetImpl( NativeCommandList* cmdList )
{
    auto nativeDeviceContext = cmdList->deferredContext;
    if ( cmdList->currentDepthBuffer != nullptr ) {
        nativeDeviceContext->ClearDepthStencilView( cmdList->currentDepthBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, cmdList->depthClearValue, 0x0 );
    }
}

void flan::rendering::DrawImpl( NativeCommandList* cmdList, const unsigned int vertexCount, const unsigned int vertexOffset )
{
    cmdList->deferredContext->Draw( vertexCount, vertexOffset );
}

void flan::rendering::DrawIndexedImpl( NativeCommandList* cmdList, const uint32_t indiceCount, const uint32_t indiceOffset, const std::size_t indiceType, const uint32_t vertexOffset )
{
    cmdList->deferredContext->DrawIndexed( indiceCount, indiceOffset, vertexOffset );
}

void flan::rendering::DrawInstancedIndexedImpl( NativeCommandList* cmdList, const unsigned int indiceCount, const unsigned int instanceCount, const unsigned int indexOffset, const unsigned int vertexOffset, const unsigned int instanceOffset )
{
    cmdList->deferredContext->DrawIndexedInstanced( indiceCount, instanceCount, indexOffset, vertexOffset, instanceOffset );
}

void flan::rendering::DispatchComputeImpl( NativeCommandList* cmdList, const unsigned int threadCountX, const unsigned int threadCountY, const unsigned int threadCountZ )
{
    cmdList->deferredContext->Dispatch( threadCountX, threadCountY, threadCountZ );
}

void flan::rendering::GetViewportImpl( NativeCommandList* cmdList, Viewport& viewport )
{
    // TODO Support multiple viewport (if it comes useful at some point)
    UINT viewportCount = 1;
    D3D11_VIEWPORT activeViewport = {};
    cmdList->deferredContext->RSGetViewports( &viewportCount, &activeViewport );

    viewport.X = static_cast<int32_t>( activeViewport.TopLeftX );
    viewport.Y = static_cast<int32_t>( activeViewport.TopLeftY );

    viewport.Width = static_cast<int32_t>( activeViewport.Width );
    viewport.Height = static_cast<int32_t>( activeViewport.Height );

    viewport.MinDepth = activeViewport.MinDepth;
    viewport.MaxDepth = activeViewport.MaxDepth;
}

void flan::rendering::SetViewportImpl( NativeCommandList* cmdList, const Viewport& viewport )
{
    const D3D11_VIEWPORT d3dViewport =
    {
        static_cast<FLOAT>( viewport.X ),
        static_cast<FLOAT>( viewport.Y ),
        static_cast<FLOAT>( viewport.Width ),
        static_cast<FLOAT>( viewport.Height ),
        viewport.MinDepth,
        viewport.MaxDepth,
    };

    cmdList->deferredContext->RSSetViewports( 1, &d3dViewport );
}

void flan::rendering::BindBackbufferImpl( NativeCommandList* cmdList )
{
    cmdList->currentRenderTargetCount = 1;
    cmdList->currentRenderTargets[0] = cmdList->backbuffer;

    for ( int i = 1; i < 8; i++ )
        cmdList->currentRenderTargets[i] = nullptr;

    cmdList->currentDepthBuffer = nullptr;

    cmdList->deferredContext->OMSetRenderTargets( cmdList->currentRenderTargetCount, cmdList->currentRenderTargets, cmdList->currentDepthBuffer );
}

void flan::rendering::BindRenderTargetImpl( NativeCommandList* cmdList, RenderTarget** renderTarget, RenderTarget* depthRenderTarget, const uint32_t renderTargetCount, const uint32_t mipLevel )
{
    BindRenderTargetLayeredImpl( cmdList, renderTarget, depthRenderTarget, renderTargetCount, mipLevel, 0 );
}

void flan::rendering::BindRenderTargetLayeredImpl( NativeCommandList* cmdList, RenderTarget** renderTarget, RenderTarget* depthRenderTarget, const uint32_t renderTargetCount, const uint32_t mipLevel, const uint32_t layerIndex )
{
    cmdList->currentRenderTargetCount = renderTargetCount;

    for ( unsigned int targetId = 0; targetId < renderTargetCount; targetId++ ) {
        auto nativeRenderTarget = renderTarget[targetId]->getNativeRenderTargetObject();
        cmdList->currentRenderTargets[targetId] = nativeRenderTarget->textureRenderTargetViewPerSliceAndMipLevel[layerIndex][mipLevel];
    }

    if ( depthRenderTarget != nullptr ) {
        auto nativeDepthTarget = depthRenderTarget->getNativeRenderTargetObject();
        cmdList->currentDepthBuffer = nativeDepthTarget->textureDepthStencilViewPerSliceAndMipLevel[layerIndex][mipLevel];
    } else {
        cmdList->currentDepthBuffer = nullptr;
    }

    cmdList->deferredContext->OMSetRenderTargets( cmdList->currentRenderTargetCount, cmdList->currentRenderTargets, cmdList->currentDepthBuffer );
}

void flan::rendering::UnbindVertexArrayImpl( NativeCommandList* cmdList )
{

}

void flan::rendering::BindPipelineStateImpl( NativeCommandList* cmdList, PipelineState* pipelineState )
{
    auto pso = pipelineState->getNativePipelineStateObject();
    auto nativeDeviceContext = cmdList->deferredContext;

    nativeDeviceContext->IASetPrimitiveTopology( pso->primitiveTopology );
    nativeDeviceContext->IASetInputLayout( pso->inputLayout );

    nativeDeviceContext->VSSetShader( pso->vertexStage, nullptr, 0 );
    nativeDeviceContext->HSSetShader( pso->tesselationControlStage, nullptr, 0 );
    nativeDeviceContext->DSSetShader( pso->tesselationEvalStage, nullptr, 0 );
    nativeDeviceContext->PSSetShader( pso->pixelStage, nullptr, 0 );
    nativeDeviceContext->CSSetShader( pso->computeStage, nullptr, 0 );
}
#endif
