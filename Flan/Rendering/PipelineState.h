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

class RenderDevice;
class Shader;
struct NativePipelineStateObject;
class DepthStencilState;
class BlendState;
class RasterizerState;

#include "PrimitiveTopologies.h"
#include <vector>
#include "ImageFormat.h"

struct InputLayoutEntry
{
    uint32_t        index;
    eImageFormat    format;
    uint32_t        instanceCount;
    uint32_t        vertexBufferIndex;
    uint32_t        offsetInBytes;
    bool            needPadding;
    const char*     semanticName;
};

using InputLayout_t = std::vector<InputLayoutEntry>;

struct PipelineStateDesc
{
    PipelineStateDesc()
        : vertexStage( nullptr )
        , tesselationControlStage( nullptr )
        , tesselationEvalStage( nullptr )
        , pixelStage( nullptr )
        , computeStage( nullptr )
        , depthStencilState( nullptr )
        , blendState( nullptr )
        , rasterizerState( nullptr )
        , primitiveTopology( flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST )
        , inputLayout{}
    {
        inputLayout.reserve( 2 );
    }

    Shader*                             vertexStage;
    Shader*                             tesselationControlStage;
    Shader*                             tesselationEvalStage;
    Shader*                             pixelStage;
    Shader*                             computeStage;

    DepthStencilState*                  depthStencilState;
    BlendState*                         blendState;
    RasterizerState*                    rasterizerState;
    
    flan::rendering::ePrimitiveTopology primitiveTopology;
    InputLayout_t                       inputLayout;
};

class PipelineState
{
public:
                PipelineState();
                PipelineState( PipelineState& ) = delete;
                PipelineState& operator = ( PipelineState& ) = delete;
                ~PipelineState();

    void        create( RenderDevice* renderDevice, const PipelineStateDesc& description );
    void        destroy( RenderDevice* renderDevice );

    NativePipelineStateObject* getNativePipelineStateObject() const;
    const PipelineStateDesc* getDescription() const;

private:
    PipelineStateDesc                          pipelineStateDescription;
    std::unique_ptr<NativePipelineStateObject> nativePipelineStateObject;
};
