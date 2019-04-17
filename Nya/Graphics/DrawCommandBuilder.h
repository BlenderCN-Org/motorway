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
class WorldRenderer;
class Mesh;
class VertexArrayObject;
class LightGrid;
class PoolAllocator;
class StackAllocator;
class Material;
class GraphicsAssetCache;

struct CameraData;
struct IBLProbeData;

#include <stack>

#include <Maths/Vector.h>
#include <Maths/Matrix.h>

enum eProbeCaptureStep : uint16_t
{
    FACE_X_PLUS = 0,
    FACE_X_MINUS,

    FACE_Y_PLUS,
    FACE_Y_MINUS,

    FACE_Z_PLUS,
    FACE_Z_MINUS,
};

class DrawCommandBuilder
{
// TODO Cleaner way to store debug/devbuild specific resources?
#if NYA_DEVBUILD
public:
    Material*                   MaterialDebugIBLProbe;
#endif

public:
                                DrawCommandBuilder( BaseAllocator* allocator );
                                DrawCommandBuilder( DrawCommandBuilder& ) = delete;
                                DrawCommandBuilder& operator = ( DrawCommandBuilder& ) = delete;
                                ~DrawCommandBuilder();

#if NYA_DEVBUILD
    void                        loadDebugResources( GraphicsAssetCache* graphicsAssetCache );
#endif

    void                        addGeometryToRender( const Mesh* meshResource, const nyaMat4x4f* modelMatrix, const uint32_t flagset );
    void                        addSphereToRender( const nyaVec3f& sphereCenter, const float sphereRadius, Material* material );
    void                        addCamera( CameraData* cameraData );
    void                        addIBLProbeToCapture( const IBLProbeData* probeData );
    void                        addHUDRectangle( const nyaVec2f& positionScreenSpace, const nyaVec2f& dimensionScreenSpace, const float rotationInRadians, Material* material );

    void                        addHUDText( const nyaVec2f& positionScreenSpace, const float size, const nyaVec4f& colorAndAlpha, const std::string& value );

    void                        buildRenderQueues( WorldRenderer* worldRenderer, LightGrid* lightGrid );

private:
    struct MeshInstance {
        const Mesh*         mesh;
        const nyaMat4x4f*   modelMatrix;

        // TODO Unify with Scene mesh instance flagset
        union {
            struct {
                uint8_t isVisible : 1;
                uint8_t renderDepth : 1;
                uint8_t useBatching : 1;
                uint8_t : 0;
            };

            uint32_t    flags;
        };
    };

    struct IBLProbeCaptureCommand {
        union {
            // Higher bytes contains probe index (from the lowest to the highest one available in the queue)
            // Lower bytes contains face to render (not used in queue sort)
            struct {
                uint32_t            EnvProbeArrayIndex;
                eProbeCaptureStep   Step;
                uint16_t            __PADDING__;
            } CommandInfos;

            int64_t ProbeKey;
        };

        const IBLProbeData*   Probe;

        inline bool operator < ( const IBLProbeCaptureCommand& cmd ) {
            return ProbeKey < cmd.ProbeKey;
        }
    };

    struct IBLProbeConvolutionCommand {
        union {
            // Higher bytes contains probe index (from the lowest to the highest one available in the queue)
            // Lower bytes contains mip index (with padding)
            struct {
                uint32_t            EnvProbeArrayIndex;
                eProbeCaptureStep   Step;
                uint16_t            MipIndex;
            } CommandInfos;

            int64_t ProbeKey;
        };

        const IBLProbeData* Probe;

        inline bool operator < ( const IBLProbeConvolutionCommand& cmd ) {
            return ProbeKey < cmd.ProbeKey;
        }
    };

    struct PrimitiveInstance {
        nyaMat4x4f      modelMatrix;
        Material*       material;
    };

    struct TextDrawCommand
    {
        std::string     stringToPrint;
        nyaVec4f        color;
        nyaVec2f        positionScreenSpace;
        float           scale;
    };

private:
    BaseAllocator*                          memoryAllocator;

    PoolAllocator*                          cameras;
    PoolAllocator*                          meshes;
    PoolAllocator*                          spheresToRender;
    nyaMat4x4f                              sphereToRender[8192];

    PoolAllocator*                          primitivesToRender;
    nyaMat4x4f                              primitivesModelMatricess[8192];

    PoolAllocator*                          textToRenderAllocator;

    StackAllocator*                         probeCaptureCmdAllocator;
    StackAllocator*                         probeConvolutionCmdAllocator;

    std::stack<IBLProbeCaptureCommand*>     probeCaptureCmds;
    std::stack<IBLProbeConvolutionCommand*> probeConvolutionCmds;

private:
    void                        resetEntityCounters();
    void                        buildMeshDrawCmds( WorldRenderer* worldRenderer, CameraData* camera, const uint8_t cameraIdx, const uint8_t layer, const uint8_t viewportLayer );
    void                        buildHUDDrawCmds( WorldRenderer* worldRenderer, CameraData* camera, const uint8_t cameraIdx );
};
