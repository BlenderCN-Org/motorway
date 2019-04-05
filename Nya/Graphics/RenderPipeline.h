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

#include <functional>
#include <map>

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include <Framework/Cameras/Camera.h>

#include "WorldRenderer.h"

class GraphicsProfiler;
class RenderPipelineBuilder;
class RenderPipelineResources;
class RenderDevice;
class CommandList;

struct Buffer;
struct BufferDesc;
struct RenderTarget;
struct TextureDescription;
struct CameraData;
struct DrawCmd;

using ResHandle_t = uint32_t;
using MutableResHandle_t = uint32_t;

template<typename T>
using nyaPassSetup_t = std::function< void( RenderPipelineBuilder&, T& ) >;

template<typename T>
using nyaPassCallback_t = std::function< void( const T&, const RenderPipelineResources&, RenderDevice* ) >;

using BindCallback = std::function< void( const RenderPipelineResources&, RenderDevice* ) >;

struct RenderPipelineRenderPass
{
    char                data[1024];
    BindCallback        execute;
    std::string         name;
};

class RenderPipelineBuilder
{
public:
    enum eRenderTargetFlags {
        USE_PIPELINE_DIMENSIONS     = 1 << 0, // Use builder viewport dimensions (override width/height)
        USE_PIPELINE_SAMPLER_COUNT  = 1 << 1, // Use builder sampler count (override samplerCount)
    };

    enum eRenderTargetCopyFlags {
        NO_MULTISAMPLE = 1 << 0, // Copy description with sampler count set to 1
    };

public:
                RenderPipelineBuilder();
                RenderPipelineBuilder( RenderPipelineBuilder& ) = default;
                RenderPipelineBuilder& operator = ( RenderPipelineBuilder& ) = default;
                ~RenderPipelineBuilder();

    void        compile( RenderDevice* renderDevice, RenderPipelineResources& resources );
    void        cullRenderPasses( RenderPipelineRenderPass* renderPassList, int& renderPassCount );

    void        useAsyncCompute( const bool state );
    void        setUncullablePass();

    void        addRenderPass();
    void        setPipelineViewport( const Viewport& viewport );
    void        setMSAAQuality( const uint32_t samplerCount = 1 );
    void        setImageQuality( const float imageQuality = 1.0f );

    ResHandle_t allocateRenderTarget( TextureDescription& description, const uint32_t flags = 0 );
    ResHandle_t copyRenderTarget( const ResHandle_t resourceToCopy, const uint32_t copyFlags = 0 );
    ResHandle_t allocateBuffer( BufferDesc& description, const uint32_t shaderStageBinding, const uint32_t flags = 0 );
    ResHandle_t allocateSampler( const SamplerDesc& description );

    ResHandle_t readRenderTarget( const ResHandle_t resourceHandle );
    ResHandle_t readBuffer( const ResHandle_t resourceHandle );

    ResHandle_t retrievePersistentRenderTarget( const nyaStringHash_t resourceHashcode );
    ResHandle_t retrievePersistentBuffer( const nyaStringHash_t resourceHashcode );

private:
    Viewport    pipelineViewport;
    uint32_t    pipelineSamplerCount;
    float       pipelineImageQuality;

    struct {
        uint32_t renderTargets[48];
        uint32_t renderTargetCount;
        uint32_t buffers[48];
        uint32_t buffersCount;
        bool     isUncullable;
    } passRefs[48];
    int32_t     renderPassCount;

    struct {
        TextureDescription  description;
        uint32_t            flags;
        uint32_t            referenceCount;
    } renderTargets[48];
    uint32_t renderTargetCount;

    struct {
        BufferDesc  description;
        uint32_t    shaderStageBinding;
        uint32_t    referenceCount;
    } buffers[48]; 
    uint32_t bufferCount;

    nyaStringHash_t persitentBuffers[48];
    uint32_t persitentBufferCount;

    nyaStringHash_t persitentRenderTargets[48];
    uint32_t persitentRenderTargetCount;

    SamplerDesc samplers[48];
    uint32_t samplerCount;
};

class RenderPipelineResources
{
public:
    struct DrawCmdBucket {
        DrawCmd* beginAddr;
        DrawCmd* endAddr;

        float    vectorPerInstance;
        float    instanceDataStartOffset;

        DrawCmd* begin() {
            return beginAddr;
        }

        const DrawCmd* begin() const {
            return beginAddr;
        }

        DrawCmd* end() {
            return endAddr;
        }

        const DrawCmd* end() const {
            return endAddr;
        }
    };

public:
                            RenderPipelineResources();
                            RenderPipelineResources( RenderPipelineResources& ) = default;
                            RenderPipelineResources& operator = ( RenderPipelineResources& ) = default;
                            ~RenderPipelineResources();

    void                    create( BaseAllocator* allocator );
    void                    destroy( BaseAllocator* allocator );

    void                    releaseResources( RenderDevice* renderDevice );
    void                    unacquireResources();

    void                    setPipelineViewport( const Viewport& viewport, const CameraData* cameraData );
    void                    setImageQuality( const float imageQuality = 1.0f );

    void                    dispatchToBuckets( DrawCmd* drawCmds, const size_t drawCmdCount );
    void                    importPersistentRenderTarget( const nyaStringHash_t resourceHashcode, RenderTarget* renderTarget );
    void                    importPersistentBuffer( const nyaStringHash_t resourceHashcode, Buffer* buffer );

    const DrawCmdBucket&    getDrawCmdBucket( const DrawCommandKey::Layer layer, const uint8_t viewportLayer ) const;
    void*                   getVectorBufferData() const;

    const CameraData*       getMainCamera() const;
    const Viewport*         getMainViewport() const;

    void                    updateDeltaTime( const float dt );
    const float             getDeltaTime() const;

    Buffer*                 getBuffer( const ResHandle_t resourceHandle ) const;
    RenderTarget*           getRenderTarget( const ResHandle_t resourceHandle ) const;
    Sampler*                getSampler( const ResHandle_t resourceHandle ) const;

    // TODO Find a clean way to merge persitent getters with regular resource getters?
    Buffer*                 getPersistentBuffer( const ResHandle_t resourceHandle ) const;
    RenderTarget*           getPersitentRenderTarget( const ResHandle_t resourceHandle ) const;

    void                    allocateBuffer( RenderDevice* renderDevice, const ResHandle_t resourceHandle, const BufferDesc& description );
    void                    allocateRenderTarget( RenderDevice* renderDevice, const ResHandle_t resourceHandle, const TextureDescription& description );
    void                    allocateSampler( RenderDevice* renderDevice, const ResHandle_t resourceHandle, const SamplerDesc& description );

    void                    bindPersistentBuffers( const ResHandle_t resourceHandle, const nyaStringHash_t hashcode );
    void                    bindPersistentRenderTargets( const ResHandle_t resourceHandle, const nyaStringHash_t hashcode );

    bool                    isPersistentRenderTargetAvailable( const nyaStringHash_t resourceHashcode ) const;
    bool                    isPersistentBufferAvailable( const nyaStringHash_t resourceHashcode ) const;

private:
    void*                   instanceBufferData;

    Buffer*                 cbuffers[96];
    size_t                  cbuffersSize[96];
    bool                    isCBufferFree[96];

    int                     cbufferAllocatedCount;

    Buffer*                 genBuffer[96];
    BufferDesc              genBufferDesc[96];
    bool                    isGenBufferFree[96];

    int                     genAllocatedCount;

    Buffer*                 uavTex2dBuffer[96];
    BufferDesc              uavTex2dBufferDesc[96];
    bool                    isUavTex2dBufferFree[96];

    int                     uavTex2dAllocatedCount;

    Buffer*                 uavBuffer[96];
    BufferDesc              uavBufferDesc[96];
    bool                    isUavBufferFree[96];

    int                     uavBufferAllocatedCount;

    RenderTarget*           renderTargets[96];
    TextureDescription      renderTargetsDesc[96];
    bool                    isRenderTargetAvailable[96];

    int                     rtAllocatedCount;

    Sampler*                samplers[96];
    SamplerDesc             samplersDesc[96];
    bool                    isSamplerAvailable[96];
    int                     samplerAllocatedCount;

    Buffer*                 allocatedBuffers[48];
    RenderTarget*           allocatedRenderTargets[48];
    Sampler*                allocatedSamplers[48];

    Buffer*                 allocatedPersistentBuffers[48];
    RenderTarget*           allocatedPersistentRenderTargets[48];

    DrawCmdBucket           drawCmdBuckets[4][8];
    CameraData              activeCameraData;
    Viewport                activeViewport;
    float                   pipelineImageQuality;
    float                   deltaTime;

    std::map<nyaStringHash_t, Buffer*>          persistentBuffers;
    std::map<nyaStringHash_t, RenderTarget*>    persistentRenderTarget;

private:
    void                    updateVectorBuffer( const DrawCmd& cmd, size_t& instanceBufferOffset );
};

class RenderPipeline
{
public:
            RenderPipeline( BaseAllocator* allocator );
            RenderPipeline( RenderPipeline& ) = default;
            RenderPipeline& operator = ( RenderPipeline& ) = default;
            ~RenderPipeline();

    void    destroy( RenderDevice* renderDevice );
    void    enableProfiling( RenderDevice* renderDevice );

    void    beginPassGroup();
    void    execute( RenderDevice* renderDevice, const float deltaTime );

    void    submitAndDispatchDrawCmds( DrawCmd* drawCmds, const size_t drawCmdCount );
    void    setViewport( const Viewport& viewport, const CameraData* camera = nullptr );
    void    setMSAAQuality( const uint32_t samplerCount = 1 );

    // imageQuality = ( image Quality In Percent / 100.0f )
    // (e.g. 1.0f = 100% image quality)
    void    setImageQuality( const float imageQuality = 1.0f );

    void    importPersistentRenderTarget( const nyaStringHash_t resourceHashcode, RenderTarget* renderTarget );
    void    importPersistentBuffer( const nyaStringHash_t resourceHashcode, Buffer* buffer );

    template<typename T>
    T& addRenderPass( const std::string& name, nyaPassSetup_t<T> setup, nyaPassCallback_t<T> execute ) {
        static_assert( sizeof( T ) <= sizeof( ResHandle_t ) * 128, "Pass data 128 resource limit hit!" );
        static_assert( sizeof( execute ) <= 1024 * 1024, "Execute lambda should be < 1ko!" );

        auto& renderPass = renderPasses[renderPassCount++];
        T& passData = *( T* )renderPass.data;

        renderPipelineBuilder.addRenderPass();
        setup( renderPipelineBuilder, passData );

        renderPass.execute = std::bind(
            execute,
            passData,
            std::placeholders::_1,
            std::placeholders::_2
        );
        renderPass.name = name;

        return passData;
    }

#if NYA_DEVBUILD
    const char* getProfilingSummary() const;
#endif

private:
    BaseAllocator*                      memoryAllocator;
    RenderPipelineRenderPass            renderPasses[48];

    int                                 renderPassCount;

    uint32_t                            passGroupStartIndexes[8];
    int                                 passGroupCount;

    Viewport                            activeViewport;
    bool                                hasViewportChanged;
    float                               pipelineImageQuality;

    // Persistent Resources
    RenderTarget*                       lastFrameRenderTarget;

    RenderPipelineResources             renderPipelineResources;
    RenderPipelineBuilder               renderPipelineBuilder;

    GraphicsProfiler*                   graphicsProfiler;
};
