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

#include <Rendering/Texture.h>
#include <Rendering/PipelineState.h>
#include <Rendering/Buffer.h>
#include <Rendering/DepthStencilState.h>
#include <Rendering/BlendState.h>
#include <Rendering/RasterizerState.h>
#include <Rendering/Viewport.h>

// REFACTOR Simplify Camera::Data > WorldViewport (matrices might be hand written for special cases and not bound to
// a camera entity)
#include <Framework/Cameras/Camera.h>

class RenderDevice;
class RenderPipeline;
class RenderPipelineBuilder;
class RenderPipelineResources;
struct Viewport;

using fnPipelineResHandle_t = uint32_t;
using fnPipelineMutableResHandle_t = uint32_t;

struct RenderPassData
{
    fnPipelineMutableResHandle_t output[128];
    fnPipelineMutableResHandle_t input[128];

    fnPipelineResHandle_t        pipelineState;
    fnPipelineResHandle_t        buffers[32];
    fnPipelineResHandle_t        samplers[16];
};

using fnRenderPassSetup_t = std::function< void( RenderPipelineBuilder*, RenderPassData& ) >;
using fnRenderPassPipelineSetup_t = std::function< void( RenderPipeline*, RenderPipelineBuilder* ) >;
using fnRenderPassCallback_t = std::function< void( CommandList*, const RenderPipelineResources*, const RenderPassData& ) >;

struct RenderPass
{
    std::string             name;
    fnRenderPassSetup_t     setup;
    fnRenderPassCallback_t  execute;
    RenderPassData          data;
};

struct RenderPassPipelineStateDesc
{
    RenderPassPipelineStateDesc()
        : hashcode()
        , vertexStage( FLAN_STRING( "" ) )
        , tesselationControlStage( FLAN_STRING( "" ) )
        , tesselationEvalStage( FLAN_STRING( "" ) )
        , pixelStage( FLAN_STRING( "" ) )
        , computeStage( FLAN_STRING( "" ) )
        , depthStencilState()
        , blendState()
        , rasterizerState()
        , primitiveTopology( flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST )
    {

    }

    fnStringHash_t                      hashcode;
    fnString_t                          vertexStage;
    fnString_t                          tesselationControlStage;
    fnString_t                          tesselationEvalStage;
    fnString_t                          pixelStage;
    fnString_t                          computeStage;
    DepthStencilStateDesc               depthStencilState;
    BlendStateDesc                      blendState;
    RasterizerStateDesc                 rasterizerState;
    flan::rendering::ePrimitiveTopology primitiveTopology;
    InputLayout_t                       inputLayout;
};

struct RenderPassTextureDesc
{
    RenderPassTextureDesc()
        : initialState( RenderPassTextureDesc::DONT_CARE )
        , resourceToCopy( -1 )
        , useGlobalDimensions( false )
        , useGlobalMultisamplingState( false )
        , copyResource( false )
        , description()
        , lamdaComputeDimension( nullptr )
    {

    }

    enum : uint32_t
    {
        DONT_CARE = 0,
        CLEAR = 1,
        CLEAR_DEPTH_REVERSED_Z
    } initialState;

    fnPipelineMutableResHandle_t resourceToCopy;

    bool useGlobalDimensions;
    bool useGlobalMultisamplingState;
    bool copyResource;

    TextureDescription description;
    std::function<glm::uvec2( uint32_t, uint32_t )> lamdaComputeDimension;
};

struct RenderPipelineViewport
{
    Viewport                        rendererViewport;
    Camera::Data                    worldViewport;
    std::vector<fnStringHash_t>     renderPasses;
    std::map<fnStringHash_t, void*> renderPassesArgs;
};

#define FLAN_REGISTER_RENDERPASS( passName, passFunc )\
static int passName##PassRegisteration =\
Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( FLAN_STRING_HASH( #passName ), [=]( RenderPipeline* renderPipeline ) { return passFunc( renderPipeline ); } );

// Same as FLAN_REGISTER_RENDERPASS; except you can provide a custom lambda function to invoc your render pass (in case your render pass have parameters)
#define FLAN_REGISTER_RENDERPASS_CUSTOM_INVOC( passName, passFuncInvoc )\
static int passName##PassRegisteration =\
Factory<fnPipelineResHandle_t, RenderPipeline*>::registerComponent( FLAN_STRING_HASH( #passName ), passFuncInvoc );
