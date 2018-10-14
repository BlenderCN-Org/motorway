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

#include <array>
#include <vector>

#include <Core/Maths/Frustum.h>
#include <Shaders/ShadowMappingShared.h>

FLAN_DEV_VAR( TAAJitteringScale,
    "Defines Temporal Antialiasing jittering (applied to the projection matrix) scale [0.0..1.0]",
    0.05f,
    float )

//=====================================
//  Camera
//      Object holding generic informations about
//      a camera (world position, matrix configuration,
//      ...).
//      
//      Since the struct is uploaded as is on the GPU,
//      it should always be 16 bytes aligned.
//=====================================
class Camera
{
public:
    struct Data
    {
        glm::mat4	viewMatrix;
        glm::mat4	projectionMatrix;
        glm::mat4	inverseViewMatrix;
        glm::mat4	inverseProjectionMatrix;
        glm::mat4	viewProjectionMatrix;
        glm::mat4	inverseViewProjectionMatrix;
        glm::vec3	worldPosition;
        int32_t 	cameraFrameNumber;

        // Shadow mapping rendering specifics       
        glm::mat4	depthProjectionMatrix;
        glm::mat4	depthViewProjectionMatrix;
        glm::vec4   cascadeOffsets[CSM_SLICE_COUNT];
        glm::vec4   cascadeScales[CSM_SLICE_COUNT];
        float       cascadeSplitDistances[CSM_SLICE_COUNT];
        glm::mat4   shadowViewMatrix[CSM_SLICE_COUNT];
        glm::mat4   globalShadowMatrix;

        // Temporal infos
        glm::mat4	previousViewProjectionMatrix;
        glm::mat4	previousViewMatrix;
        glm::vec2   jitteringOffset;
        glm::vec2   previousJitteringOffset;

        Frustum     frustum;
    };
    FLAN_IS_MEMORY_ALIGNED( 16, Data );

public:
    inline const Data&                          GetData() const { return data; }
    inline Data&                                GetDataRW() { return data; }
    inline const std::vector<fnStringHash_t>*   getRenderPassList() const { return &renderPasses; }
    inline const Frustum*                       getFrustum() const { return &data.frustum; }

public:
    Camera()
        : data{ }
    {
        renderPasses.reserve( 128 );
    }

    virtual void Update( float frameTime ) = 0;
    virtual void DrawInEditor( float frameTime ) = 0;
    virtual void Serialize( FileSystemObject* stream ) = 0;
    virtual void Deserialize( FileSystemObject* stream ) = 0;

    void addRenderPass( const fnStringHash_t renderPassHashcode, void* renderPassArgs = nullptr )
    {
        if ( renderPasses.size() >= 128 ) {
            FLAN_CERR << "Too many render passes for this camera!" << std::endl;
            return;
        }

        renderPasses.push_back( renderPassHashcode );
        renderPassesArgs[renderPassHashcode] = renderPassArgs;
    }

    bool isUsingRenderPass( const fnStringHash_t renderPassHashcode ) const
    {
        return std::find( renderPasses.begin(), renderPasses.end(), renderPassHashcode ) != renderPasses.end();
    }

    void clearRenderPasses()
    {
        renderPasses.clear();
        renderPassesArgs.clear();
    }

protected:
    Data                                data;
    std::vector<fnStringHash_t>         renderPasses;
    std::map<fnStringHash_t, void*>     renderPassesArgs;
};
