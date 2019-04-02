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

#if NYA_D3D11
#include <d3d11.h>

static constexpr int MAX_RES_COUNT = 64;

struct PipelineState
{
    ID3D11VertexShader*                 vertexShader;
    ID3D11HullShader*                   tesselationControlShader;
    ID3D11DomainShader*                 tesselationEvalShader;
    ID3D11PixelShader*                  pixelShader;
    ID3D11ComputeShader*                computeShader;
    ID3D11InputLayout*                  inputLayout;
    D3D11_PRIMITIVE_TOPOLOGY            primitiveTopology;
    ID3D11BlendState*                   blendState;
    UINT                                blendMask;
    ID3D11DepthStencilState*            depthStencilState;
    UINT                                stencilRef;
    ID3D11RasterizerState*              rasterizerState;

    struct ResourceListLayout {
        enum ResourceBindType
        {
            Buffer,
            Sampler,
            Texture,
            RenderTarget,
            UAVResource
        };

        struct {
            int resourceIndex;
            ResourceBindType type;
            union {
                ID3D11ShaderResourceView**   shaderResourceView;
                ID3D11UnorderedAccessView**  unorderedAccessView;
                ID3D11SamplerState**         samplerState;
                ID3D11Buffer**               buffers;
            };
        } resources[64 * 4];

        int resourceToBindCount;

        template<typename Res>
        struct ResourceBinding
        {
            Res  vertexStage[MAX_RES_COUNT];
            Res  hullStage[MAX_RES_COUNT];
            Res  domainStage[MAX_RES_COUNT];
            Res  pixelStage[MAX_RES_COUNT];
            Res  computeStage[MAX_RES_COUNT];
        };

        ResourceBinding<ID3D11ShaderResourceView*>  shaderResourceViews;
        ID3D11UnorderedAccessView*                  uavBuffers[MAX_RES_COUNT];
        UINT                                        uavBuffersBindCount;
        ResourceBinding<ID3D11Buffer*>              constantBuffers;
        ResourceBinding<ID3D11SamplerState*>        samplers;
    } resourceList;

    struct RenderPassLayout
    {
        enum AttachmentType
        {
            RenderTargetView,
            DepthStencilView,
            ShaderResourceView
        };

        struct
        {
            int resourceIndex;
            AttachmentType type;

            union {
                ID3D11RenderTargetView**     renderTargetView;
                ID3D11DepthStencilView**     depthStencilView;
                ID3D11ShaderResourceView**   shaderResourceView;
            };
        } resources[24 * 2];
        int resourceToBindCount;

        ID3D11RenderTargetView* renderTargetViews[8];
        ID3D11DepthStencilView* depthStencilView;

        FLOAT clearValue[8 + 1][4];
        bool clearTarget[8 + 1];
        UINT rtvCount;

        struct SRVStageBind {
            ID3D11ShaderResourceView* shaderResourceView[16];
            UINT srvCount;
        };

        SRVStageBind    pixelStage;
        SRVStageBind    computeStage;
    } renderPass;
};

#endif
