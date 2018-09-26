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

class Material;
class VertexArrayObject;
struct EnvironmentProbe;

struct DrawCommandKey
{
    enum Layer : uint8_t {
        LAYER_DEPTH,
        LAYER_WORLD,
        LAYER_HUD,
        LAYER_DEBUG
    };

    enum DepthViewportLayer : uint8_t {
        DEPTH_VIEWPORT_LAYER_DEFAULT,
        DEPTH_VIEWPORT_LAYER_CSM0,
        DEPTH_VIEWPORT_LAYER_CSM1,
        DEPTH_VIEWPORT_LAYER_CSM2,
        DEPTH_VIEWPORT_LAYER_CSM3
    };

    enum WorldViewportLayer : uint8_t {
        WORLD_VIEWPORT_LAYER_DEFAULT,
    };

    enum DebugViewportLayer : uint8_t {
        DEBUG_VIEWPORT_LAYER_DEFAULT,
    };
    
    enum HUDViewportLayer : uint8_t {
        HUD_VIEWPORT_LAYER_DEFAULT,
    };

    enum SortOrder : uint8_t {
        SORT_FRONT_TO_BACK = 0,
        SORT_BACK_TO_FRONT
    };

    union
    {
        struct
        {
            uint32_t materialSortKey; // material sort key (contains states and pipeline setup infos as a bitfield)

            uint16_t depth; // half float depth for distance sorting
            SortOrder sortOrder : 2; // front to back or back to front (opaque or transparent)
            uint8_t __PLACEHOLDER__ : 6;

            Layer layer : 3;
            uint8_t viewportLayer : 2;
            uint8_t viewportId : 3; // viewport dispatch index (should be managed by the builder)
        } bitfield;
        uint64_t value;
    };
};

struct DrawCommandInfos
{
    const Material*             material; // Geom cmd
    const VertexArrayObject*    vao;
    unsigned int                indiceBufferOffset;
    unsigned int                indiceBufferCount;
    glm::mat4*                  modelMatrix;
};

struct DrawCommand
{
    DrawCommandKey      key;
    DrawCommandInfos    infos;

    static bool SortFrontToBack( const DrawCommand& cmd1, const DrawCommand& cmd2 )
    {
        return ( cmd1.key.value < cmd2.key.value );
    }

    static bool SortBackToFront( const DrawCommand& cmd1, const DrawCommand& cmd2 )
    {
        return ( cmd1.key.value > cmd2.key.value );
    }
};

static_assert( sizeof( DrawCommandKey ) == sizeof( uint64_t ), "DrawCommandKey is not 64bits" );

enum eProbeCaptureStep : uint16_t
{
    FACE_X_PLUS = 0,
    FACE_X_MINUS,

    FACE_Y_PLUS,
    FACE_Y_MINUS,

    FACE_Z_PLUS,
    FACE_Z_MINUS,
};

struct EnvProbeCaptureCommand
{
    union
    {
        // Higher bytes contains probe index (from the lowest to the highest one available in the queue)
        // Lower bytes contains face to render (not used in queue sort)
        struct
        {
            uint32_t            EnvProbeArrayIndex;
            eProbeCaptureStep   Step;
            uint16_t            __PADDING__;
        } CommandInfos;

        int64_t ProbeKey;
    };

    EnvironmentProbe*   Probe;

    inline bool operator < ( const EnvProbeCaptureCommand& cmd )
    {
        return ProbeKey < cmd.ProbeKey;
    }
};

struct EnvProbeConvolutionCommand
{
    union
    {
        // Higher bytes contains probe index (from the lowest to the highest one available in the queue)
        // Lower bytes contains mip index (with padding)
        struct
        {
            uint32_t            EnvProbeArrayIndex;
            eProbeCaptureStep   Step;
            uint16_t            MipIndex;
        } CommandInfos;

        int64_t ProbeKey;
    };

    EnvironmentProbe*   Probe;

    inline bool operator < ( const EnvProbeConvolutionCommand& cmd )
    {
        return ProbeKey < cmd.ProbeKey;
    }
};
