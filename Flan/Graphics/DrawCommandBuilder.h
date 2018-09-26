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

class TaskManager;
class GraphicsAssetManager;

#include "DrawCommand.h"
#include <Framework/Cameras/Camera.h>
#include <Framework/RenderKey.h>
#include <Rendering/Viewport.h>
#include <Core/Maths/BoundingSphere.h>

#include <queue>
#include <list>
#include <functional>

class Mesh;
class Transform;
class WorldRenderer;
class Camera;
class RenderableEntityManager;
class RenderDevice;
struct EnvironmentProbe;

struct MeshInstance
{
    Mesh*       meshAsset;
    Transform*  meshTransform;
};

static constexpr int MAX_VIEWPORT_COUNT = 8;

class DrawCommandBuilder
{
public:
            DrawCommandBuilder();
            DrawCommandBuilder( DrawCommandBuilder& ) = delete;
            DrawCommandBuilder& operator = ( DrawCommandBuilder& ) = delete;
            ~DrawCommandBuilder();

    void    create( TaskManager* taskManagerInstance, RenderableEntityManager* rEntityManInstance, GraphicsAssetManager* graphicsAssetManagerInstance, WorldRenderer* worldRendererInstance );

    void    addCamera( Camera* camera );
    void    addMeshToRender( MeshInstance* meshInstance );
    void    addWireframeMeshToRender( MeshInstance* meshInstance );
    void    addEntityToUpdate( const fnRenderKey_t renderKey );
    void    addEnvProbeToCapture( EnvironmentProbe* envProbe );

    void    addWireframeSphere( const glm::vec3& positionWorldSpace, const float radius, const glm::vec4& wireColor = glm::vec4( 1, 1, 0, 1 ) );

    void    addLineToRender( const glm::vec3& from, const glm::vec3& to, const float thickness, const glm::vec4& color );

    void    addHUDRectangle( 
        const glm::vec2& positionScreenSpace, 
        const glm::vec2& dimensionsScreenSpace, 
        const float rotationInRadians,
        const Material* material = nullptr );

    // TODO split into subfunctions!
    // > HUDText should implicitly use normalized coordinates (that's HUD stuff afterall...)
    // > Outlined text should draw outlined stuff
    // > Text shoudl draw text
    void    addHUDText( const std::string& text, const float scale, const float x, const float y, const float outlineThickness = 0.8f, const glm::vec4& color = glm::vec4( 1, 1, 1, 1 ), const bool useNormalizedCoordinates = true );

    void    buildCommands( RenderDevice* renderDevice, WorldRenderer* WorldRenderer );

    unsigned int getFrameNumber() const;

private:
    TaskManager*                            taskManager;
    GraphicsAssetManager*                   graphicsAssetManager;
    RenderableEntityManager*                renderableEntityManager;
    WorldRenderer*                          worldRenderer;

    MeshInstance*                           meshInstances[1024 * 16];
    int                                     meshInstancesCount;

    MeshInstance*                           wireMeshInstances[1024];
    int                                     wireMeshInstancesCount;

    std::list<Camera*>                      cameras;

    BoundingSphere                          spheres[1024];
    glm::mat4x4                             sphereMatrices[1024];
    int                                     sphereMatriceCount;

    struct HUDRectangleCmd
    {
        glm::mat4x4     modelMatrix;
        const Material* material;
    };
    HUDRectangleCmd                         hudRectangle[1024];                          
    int                                     hudRectangleMatriceCount; 

    EnvProbeCaptureCommand                  envProbeCaptureQueue[512];
    int                                     envProbeCaptureCount;

    EnvProbeConvolutionCommand              envProbeConvolutionQueue[512];
    int                                     envProbeConvolutionCount;

    std::queue<fnRenderKey_t>               renderKeysToUpdate;

private:
    void    addGeometryForViewport( const Camera::Data& worldViewport, const int viewportIndex, const uint8_t viewportLayer, const Frustum* viewportFrustum, WorldRenderer* worldRenderer );
    void    addDepthGeometryForViewport( const Camera::Data& worldViewport, const int viewportIndex, const uint8_t viewportLayer, const Frustum* viewportFrustum, WorldRenderer* worldRenderer );
    void    addDebugGeometryForViewport( const Camera::Data& worldViewport, const int viewportIndex, const uint8_t viewportLayer, const Frustum* viewportFrustum, WorldRenderer* worldRenderer );
    void    addHUDGeometryForViewport( const Camera::Data& worldViewport, const int viewportIndex, const uint8_t viewportLayer, WorldRenderer* worldRenderer );
};
