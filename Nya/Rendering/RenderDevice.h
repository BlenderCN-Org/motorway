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

struct DisplaySurface;
struct RenderContext;

struct Texture;
struct RenderTarget;
struct Buffer;
struct RenderPass;
struct PipelineState;
struct Shader;
struct Sampler;
struct ResourceList;
struct QueryPool;

class CommandListPool;
class CommandList;
class ResourceListPool;

#include <stdint.h>
#include "ImageFormat.h"

namespace nya
{
    namespace rendering
    {
        enum eSamplerAddress
        {
            SAMPLER_ADDRESS_WRAP = 0,
            SAMPLER_ADDRESS_MIRROR,
            SAMPLER_ADDRESS_CLAMP_EDGE,
            SAMPLER_ADDRESS_CLAMP_BORDER,
            SAMPLER_ADDRESS_MIRROR_ONCE,

            SAMPLER_ADDRESS_COUNT,
        };
        
        enum eSamplerFilter
        {
            SAMPLER_FILTER_POINT = 0,
            SAMPLER_FILTER_BILINEAR,
            SAMPLER_FILTER_TRILINEAR,
            SAMPLER_FILTER_ANISOTROPIC_8,
            SAMPLER_FILTER_ANISOTROPIC_16,
            SAMPLER_FILTER_COMPARISON_POINT,
            SAMPLER_FILTER_COMPARISON_BILINEAR,
            SAMPLER_FILTER_COMPARISON_TRILINEAR,
            SAMPLER_FILTER_COMPARISON_ANISOTROPIC_8,
            SAMPLER_FILTER_COMPARISON_ANISOTROPIC_16,

            SAMPLER_FILTER_COUNT
        };

        enum ePrimitiveTopology
        {
            PRIMITIVE_TOPOLOGY_TRIANGLELIST = 0,
            PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            PRIMITIVE_TOPOLOGY_LINELIST,
            PRIMITIVE_TOPOLOGY_PATCH,

            PRIMITIVE_TOPOLOGY_COUNT,
        };

        enum eComparisonFunction
        {
            COMPARISON_FUNCTION_NEVER = 0,
            COMPARISON_FUNCTION_ALWAYS,
            COMPARISON_FUNCTION_LESS,
            COMPARISON_FUNCTION_GREATER,
            COMPARISON_FUNCTION_LEQUAL,
            COMPARISON_FUNCTION_GEQUAL,
            COMPARISON_FUNCTION_NOT_EQUAL,
            COMPARISON_FUNCTION_EQUAL,
            COMPARISON_FUNCTION_COUNT
        };
    
        enum eStencilOperation
        {
            STENCIL_OPERATION_KEEP = 0,
            STENCIL_OPERATION_ZERO,
            STENCIL_OPERATION_REPLACE,
            STENCIL_OPERATION_INC,
            STENCIL_OPERATION_INC_WRAP,
            STENCIL_OPERATION_DEC,
            STENCIL_OPERATION_DEC_WRAP,
            STENCIL_OPERATION_INVERT,
            STENCIL_OPERATION_COUNT
        };

        enum eCullMode
        {
            CULL_MODE_NONE = 0,
            CULL_MODE_FRONT,
            CULL_MODE_BACK,
            CULL_MODE_FRONT_AND_BACK,
            CULL_MODE_COUNT
        };

        enum eFillMode
        {
            FILL_MODE_SOLID = 0,
            FILL_MODE_WIREFRAME,

            FILL_MODE_COUNT
        };
   
        enum eBlendSource
        {
            BLEND_SOURCE_ZERO = 0,
            BLEND_SOURCE_ONE,
            BLEND_SOURCE_SRC_COLOR,
            BLEND_SOURCE_INV_SRC_COLOR,
            BLEND_SOURCE_SRC_ALPHA,
            BLEND_SOURCE_INV_SRC_ALPHA,
            BLEND_SOURCE_DEST_ALPHA,
            BLEND_SOURCE_INV_DEST_ALPHA,
            BLEND_SOURCE_DEST_COLOR,
            BLEND_SOURCE_INV_DEST_COLOR,
            BLEND_SOURCE_SRC_ALPHA_SAT,
            BLEND_SOURCE_BLEND_FACTOR,
            BLEND_SOURCE_INV_BLEND_FACTOR,

            BLEND_SOURCE_COUNT
        };

        enum eBlendOperation
        {
            BLEND_OPERATION_ADD = 0,
            BLEND_OPERATION_SUB,
            BLEND_OPERATION_MIN,
            BLEND_OPERATION_MAX,
            BLEND_OPERATION_MUL,

            BLEND_OPERATION_COUNT
        };
    }
}

enum eQueryType
{
    QUERY_TYPE_UNKNOWN = 0,
    QUERY_TYPE_TIMESTAMP,

    QUERY_TYPE_COUNT
};

enum eShaderStage
{
    SHADER_STAGE_VERTEX = 1 << 1,
    SHADER_STAGE_PIXEL = 1 << 2,
    SHADER_STAGE_TESSELATION_CONTROL = 1 << 3,
    SHADER_STAGE_TESSELATION_EVALUATION = 1 << 4,
    SHADER_STAGE_COMPUTE = 1 << 5,

    SHADER_STAGE_COUNT = 5,

    SHADER_STAGE_ALL = ~0
};

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

struct SamplerDesc
{
    nya::rendering::eSamplerFilter         filter;
    nya::rendering::eSamplerAddress        addressU;
    nya::rendering::eSamplerAddress        addressV;
    nya::rendering::eSamplerAddress        addressW;
    nya::rendering::eComparisonFunction    comparisonFunction;
    int32_t                                minLOD;
    int32_t                                maxLOD;

    SamplerDesc()
        : filter( nya::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR )
        , addressU( nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_WRAP )
        , addressV( nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_WRAP )
        , addressW( nya::rendering::eSamplerAddress::SAMPLER_ADDRESS_WRAP )
        , comparisonFunction( nya::rendering::eComparisonFunction::COMPARISON_FUNCTION_ALWAYS )
        , minLOD( 0 )
        , maxLOD( 1000 )
    {

    }

    bool operator == ( const SamplerDesc& desc ) const
    {
        return ( addressU == desc.addressU
            && addressV == desc.addressV
            && addressW == desc.addressW
            && comparisonFunction == desc.comparisonFunction
            && filter == desc.filter
            && maxLOD == desc.maxLOD
            && minLOD == desc.minLOD );
    }
};

struct BlendStateDesc
{
    bool        writeMask[4]; // R, G, B, A
    bool        enableBlend;
    bool        useSeperateAlpha;
    bool        enableAlphaToCoverage;
    uint32_t    sampleMask = 0xffffffff;

    struct
    {
        nya::rendering::eBlendSource       source;
        nya::rendering::eBlendSource       dest;
        nya::rendering::eBlendOperation    operation;
    } blendConfColor, blendConfAlpha;
};

struct TextureDescription
{
    enum {
        DIMENSION_UNKNOWN = 0,
        DIMENSION_BUFFER,
        DIMENSION_TEXTURE_1D,
        DIMENSION_TEXTURE_2D,
        DIMENSION_TEXTURE_3D,
    } dimension;

    eImageFormat    format;
    unsigned int    width;
    unsigned int    height;
    unsigned int    depth;
    unsigned int    arraySize;
    unsigned int    mipCount;
    unsigned int    samplerCount;

    struct Flagset {
        unsigned int isCubeMap : 1;
        unsigned int isDepthResource : 1;
        unsigned int useHardwareMipGen : 1;
        unsigned int useMultisamplePattern : 1;
        unsigned int allowCPUWrite : 1;
        unsigned int : 0;

        bool operator == ( const TextureDescription::Flagset& flagset ) const
        {
            return isCubeMap == flagset.isCubeMap
                && isDepthResource == flagset.isDepthResource
                && useHardwareMipGen == flagset.useHardwareMipGen
                && useMultisamplePattern == flagset.useMultisamplePattern
                && allowCPUWrite == flagset.allowCPUWrite;
        }
    } flags;

    TextureDescription()
        : dimension( DIMENSION_UNKNOWN )
        , format( eImageFormat::IMAGE_FORMAT_UNKNOWN )
        , width( 0 )
        , height( 0 )
        , depth( 1 )
        , arraySize( 1 )
        , mipCount( 1 )
        , samplerCount( 1 )
        , flags{ 0u }
    {

    }

    bool operator == ( const TextureDescription& desc ) const
    {
        return desc.width == width
            && desc.height == height
            && desc.depth == depth
            && desc.format == format
            && desc.flags == flags
            && desc.mipCount == mipCount
            && desc.samplerCount == samplerCount;
    }
};

struct BufferDesc
{
    enum {
        UNKNOWN = 0,
        CONSTANT_BUFFER,

        VERTEX_BUFFER,
        DYNAMIC_VERTEX_BUFFER,

        INDICE_BUFFER,
        DYNAMIC_INDICE_BUFFER,

        UNORDERED_ACCESS_VIEW_BUFFER,
        UNORDERED_ACCESS_VIEW_TEXTURE_1D,
        UNORDERED_ACCESS_VIEW_TEXTURE_2D,
        UNORDERED_ACCESS_VIEW_TEXTURE_3D,

        STRUCTURED_BUFFER,
        APPEND_STRUCTURED_BUFFER,

        INDIRECT_DRAW_ARGUMENTS,
        GENERIC_BUFFER
    } type;

    eImageFormat    viewFormat; // Required by UAV only
    uint32_t        stride; // Required by vbo/ibo only

    union
    {
        struct
        {
            size_t     size;
            size_t     singleElementSize;
        };

        // UAV Textures
        struct
        {
            uint32_t        width;
            uint32_t        height;
            uint32_t        depth;
            uint32_t        mipCount;
        };
    };
};

struct RenderPassDesc
{
    enum BindMode
    {
        UNUSED = 0,
        READ,
        WRITE,
        WRITE_DEPTH
    };

    enum State
    {
        DONT_CARE = 0,
        CLEAR_COLOR,
        CLEAR_DEPTH,
        IS_TEXTURE,
        IS_UAV_TEXTURE
    };

    struct
    {
        union
        {
            RenderTarget*   renderTarget;
            Texture*        texture;
            Buffer*         buffer;
        };

        uint32_t        stageBind;
        BindMode        bindMode;
        State           targetState;
        float           clearValue[4];
        uint32_t        layerIndex;
        uint32_t        mipIndex;
    } attachements[8 + 16];
};

struct ResourceListDesc
{
    template<typename Res>
    struct ResourceList
    {
        int         bindPoint;
        uint32_t    stageBind;
        Res         resource;
    };

    ResourceList<Buffer*>    uavBuffers[64];
    ResourceList<Buffer*>    constantBuffers[64];
    ResourceList<Sampler*>   samplers[64];
    ResourceList<Buffer*>    buffers[64];
};

struct DepthStencilStateDesc
{
    bool                                    enableDepthTest;
    bool                                    enableStencilTest;
    bool                                    enableDepthWrite;
    bool                                    enableDepthBoundsTest;
    float                                   depthBoundsMin;
    float                                   depthBoundsMax;
    nya::rendering::eComparisonFunction     depthComparisonFunc;
    uint8_t                                 stencilRefValue;
    uint8_t                                 stencilWriteMask;
    uint8_t                                 stencilReadMask;

    struct
    {
        nya::rendering::eComparisonFunction    comparisonFunc;
        nya::rendering::eStencilOperation      passOp;
        nya::rendering::eStencilOperation      failOp;
        nya::rendering::eStencilOperation      zFailOp;
    } front, back;
};

struct RasterizerStateDesc
{
    nya::rendering::eCullMode   cullMode;
    nya::rendering::eFillMode   fillMode;
    float                       depthBias;
    float                       slopeScale;
    float                       depthBiasClamp;
    bool                        useTriangleCCW;
};

struct PipelineStateDesc
{
    Shader*                             vertexShader;
    Shader*                             tesselationControlShader;
    Shader*                             tesselationEvalShader;
    Shader*                             pixelShader;
    Shader*                             computeShader;
    nya::rendering::ePrimitiveTopology  primitiveTopology;
    DepthStencilStateDesc               depthStencilState;
    RasterizerStateDesc                 rasterizerState;
    BlendStateDesc                      blendState;
    InputLayoutEntry                    inputLayout[8];
};

class RenderDevice
{
public:
    RenderContext*      renderContext;

public:
                        RenderDevice( BaseAllocator* allocator );
                        RenderDevice( RenderDevice& ) = delete;
                        RenderDevice& operator = ( RenderDevice& ) = delete;
                        ~RenderDevice();

    void                create( DisplaySurface* displaySurface );
    void                enableVerticalSynchronisation( const bool enabled );
    void                present();

    const nyaChar_t*    getBackendName() const;

    CommandList&        allocateGraphicsCommandList() const;
    CommandList&        allocateComputeCommandList() const;
    void                submitCommandList( CommandList* commandList );
   
    RenderTarget*       getSwapchainBuffer();

    Texture*            createTexture1D( const TextureDescription& description, const void* initialData = nullptr, const size_t initialDataSize = 0 );
    Texture*            createTexture2D( const TextureDescription& description, const void* initialData = nullptr, const size_t initialDataSize = 0 );
    Texture*            createTexture3D( const TextureDescription& description, const void* initialData = nullptr, const size_t initialDataSize = 0 );

    RenderTarget*       createRenderTarget1D( const TextureDescription& description, Texture* initialTexture = nullptr );
    RenderTarget*       createRenderTarget2D( const TextureDescription& description, Texture* initialTexture = nullptr );
    RenderTarget*       createRenderTarget3D( const TextureDescription& description, Texture* initialTexture = nullptr );
   
    Buffer*             createBuffer( const BufferDesc& description, const void* initialData = nullptr );
    Shader*             createShader( const eShaderStage stage, const void* bytecode, const size_t bytecodeSize );
    Sampler*            createSampler( const SamplerDesc& description );

    RenderPass*         createRenderPass( const RenderPassDesc& description );
    PipelineState*      createPipelineState( const PipelineStateDesc& description );
    QueryPool*          createQueryPool( const eQueryType type, const unsigned int poolCapacity );

    ResourceList&       allocateResourceList( const ResourceListDesc& description ) const;

    bool                getQueryResult( QueryPool* queryPool, const unsigned int queryIndex, uint64_t& queryResult );
    double              convertTimestampToMs( const QueryPool* queryPool, const double timestamp );

    void                destroyTexture( Texture* texture );
    void                destroyRenderTarget( RenderTarget* renderTarget );
    void                destroyBuffer( Buffer* buffer );
    void                destroyShader( Shader* shader );
    void                destroyPipelineState( PipelineState* pipelineState );
    void                destroyRenderPass( RenderPass* renderPass );
    void                destroySampler( Sampler* sampler );
    void                destroyQueryPool( QueryPool* queryPool );

    void                setDebugMarker( Texture* texture, const char* objectName );
    void                setDebugMarker( Buffer* buffer, const char* objectName );

private:
    BaseAllocator*      memoryAllocator;
};
