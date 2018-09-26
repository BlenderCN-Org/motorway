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
#include "DrawCommandBuilder.h"

#include <Core/TaskManager.h>
#include <Framework/Mesh.h>
#include <Rendering/Viewport.h>
#include <Core/Maths/Transform.h>
#include <Rendering/RenderDevice.h>
#include <Framework/Material.h>

#include "GraphicsAssetManager.h"
#include "WorldRenderer.h"
#include "RenderableEntityManager.h"

#include <Framework/DirectionalLightHelpers.h>
#include <Core/Maths/MatrixTransformations.h>

#include <vector>

#include <Core/Profiler.h>

unsigned FloatFlip( unsigned f )
{
    unsigned mask = -int( f >> 31 ) | 0x80000000;
    return f ^ mask;
}

// Taking highest 10 bits for rough sort of floats.
// 0.01 maps to 752; 0.1 to 759; 1.0 to 766; 10.0 to 772;
// 100.0 to 779 etc. Negative numbers go similarly in 0..511 range.
uint16_t DepthToBits( const float depth )
{
    union { float f; unsigned i; } f2i;
    f2i.f = depth;
    f2i.i = FloatFlip( f2i.i ); // flip bits to be sortable
    unsigned b = f2i.i >> 22; // take highest 10 bits
    return static_cast<uint16_t>( b );
}

float DistanceToPlane( glm::vec4 vPlane, glm::vec3 vPoint )
{
    return glm::dot( glm::vec4( vPoint, 1.0 ), vPlane );
}

// Frustum cullling on a sphere. Returns > 0 if visible, <= 0 otherwise
// NOTE Infinite Z version (it implicitly skips the far plane check)
float CullSphereInfReversedZ( const Frustum* frustum, glm::vec3 vCenter, float fRadius )
{
    float dist01 = glm::min( DistanceToPlane( frustum->planes[0], vCenter ), DistanceToPlane( frustum->planes[1], vCenter ) );
    float dist23 = glm::min( DistanceToPlane( frustum->planes[2], vCenter ), DistanceToPlane( frustum->planes[3], vCenter ) );
    float dist45 = DistanceToPlane( frustum->planes[5], vCenter );

    return glm::min( glm::min( dist01, dist23 ), dist45 ) + fRadius;
}

float CullSphere( const Frustum* frustum, glm::vec3 vCenter, float fRadius )
{
    float dist01 = glm::min( DistanceToPlane( frustum->planes[0], vCenter ), DistanceToPlane( frustum->planes[1], vCenter ) );
    float dist23 = glm::min( DistanceToPlane( frustum->planes[2], vCenter ), DistanceToPlane( frustum->planes[3], vCenter ) );
    float dist45 = glm::min( DistanceToPlane( frustum->planes[4], vCenter ), DistanceToPlane( frustum->planes[5], vCenter ) );

    return glm::min( glm::min( dist01, dist23 ), dist45 ) + fRadius;
}

glm::mat4x4 GetProbeCaptureViewMatrix( const glm::vec3& probePositionWorldSpace, const eProbeCaptureStep captureStep )
{
    switch ( captureStep ) {
    case eProbeCaptureStep::FACE_X_PLUS:
        return glm::lookAtLH(
            probePositionWorldSpace,
            probePositionWorldSpace + glm::vec3( 1.0f, 0.0f, 0.0f ),
            glm::vec3( 0.0f, 1.0f, 0.0f ) );

    case eProbeCaptureStep::FACE_X_MINUS:
        return glm::lookAtLH(
            probePositionWorldSpace,
            probePositionWorldSpace + glm::vec3( -1.0f, 0.0f, 0.0f ),
            glm::vec3( 0.0f, 1.0f, 0.0f ) );

    case eProbeCaptureStep::FACE_Y_MINUS:
        return glm::lookAtLH(
            probePositionWorldSpace,
            probePositionWorldSpace + glm::vec3( 0.0f, -1.0f, 0.0f ),
            glm::vec3( 0.0f, 0.0f, 1.0f ) );

    case eProbeCaptureStep::FACE_Y_PLUS:
        return glm::lookAtLH(
            probePositionWorldSpace,
            probePositionWorldSpace + glm::vec3( 0.0f, 1.0f, 0.0f ),
            glm::vec3( 0.0f, 0.0f, -1.0f ) );

    case eProbeCaptureStep::FACE_Z_PLUS:
        return glm::lookAtLH(
            probePositionWorldSpace,
            probePositionWorldSpace + glm::vec3( 0.0f, 0.0f, 1.0f ),
            glm::vec3( 0.0f, 1.0f, 0.0f ) );

    case eProbeCaptureStep::FACE_Z_MINUS:
        return glm::lookAtLH(
            probePositionWorldSpace,
            probePositionWorldSpace + glm::vec3( 0.0f, 0.0f, -1.0f ),
            glm::vec3( 0.0f, 1.0f, 0.0f ) );

    default:
        return glm::mat4x4( 1.0f );
    }

    return glm::mat4x4( 1.0f );
}

DrawCommandBuilder::DrawCommandBuilder()
    : taskManager( nullptr )
    , graphicsAssetManager( nullptr )
    , renderableEntityManager( nullptr )
    , meshInstances{ nullptr }
    , meshInstancesCount( 0 )
    , wireMeshInstancesCount( 0 )
    , sphereMatriceCount( 0 )
    , envProbeCaptureCount( 0 )
    , envProbeConvolutionCount( 0 )
    , hudRectangleMatriceCount( 0 )
{

}

DrawCommandBuilder::~DrawCommandBuilder()
{
    taskManager = nullptr;
    graphicsAssetManager = nullptr;
}

void DrawCommandBuilder::create( TaskManager* taskManagerInstance, RenderableEntityManager* rEntityManInstance, GraphicsAssetManager* graphicsAssetManagerInstance, WorldRenderer* worldRendererInstance )
{
    taskManager = taskManagerInstance;
    graphicsAssetManager = graphicsAssetManagerInstance;
    renderableEntityManager = rEntityManInstance;
    worldRenderer = worldRendererInstance;
}

void DrawCommandBuilder::addCamera( Camera* camera )
{
    cameras.push_back( camera );
}

void DrawCommandBuilder::addMeshToRender( MeshInstance* meshInstance )
{
    meshInstances[meshInstancesCount++] = meshInstance;
}

void DrawCommandBuilder::addWireframeMeshToRender( MeshInstance* meshInstance )
{
    wireMeshInstances[wireMeshInstancesCount++] = meshInstance;
}

void DrawCommandBuilder::addEntityToUpdate( const fnRenderKey_t renderKey )
{
    renderKeysToUpdate.push( renderKey );
}

void DrawCommandBuilder::addEnvProbeToCapture( EnvironmentProbe* envProbe )
{
    FLAN_IMPORT_VAR_PTR( EnvProbeDimension, uint32_t );
    auto mipCount = flan::rendering::ComputeMipCount( *EnvProbeDimension, *EnvProbeDimension );

    // Push capture cmds
    for ( uint32_t i = 0; i < 6; i++ ) {
        EnvProbeCaptureCommand captureCmd = { 0 };
        captureCmd.CommandInfos.EnvProbeArrayIndex = envProbe->ProbeIndex;
        captureCmd.CommandInfos.Step = static_cast<eProbeCaptureStep>( i );
        captureCmd.Probe = envProbe;

        envProbeCaptureQueue[envProbeCaptureCount++] = captureCmd;

        for ( uint16_t mipIndex = 0; mipIndex < mipCount; mipIndex++ ) {
            EnvProbeConvolutionCommand convolutionCmd = { 0 };
            convolutionCmd.CommandInfos.EnvProbeArrayIndex = envProbe->ProbeIndex;
            convolutionCmd.CommandInfos.Step = static_cast<eProbeCaptureStep>( i );
            convolutionCmd.CommandInfos.MipIndex = mipIndex;

            convolutionCmd.Probe = envProbe;

            envProbeConvolutionQueue[envProbeConvolutionCount++] = convolutionCmd;
        }
    }

    envProbe->IsCaptured = true;
}

void DrawCommandBuilder::addWireframeSphere( const glm::vec3& positionWorldSpace, const float radius, const glm::vec4& wireColor )
{
    glm::mat4 translationMatrix = glm::translate( glm::mat4( 1.0 ), positionWorldSpace );
    glm::mat4 scaleMatrix = glm::scale( glm::mat4( 1.0 ), glm::vec3( radius ) );

    spheres[sphereMatriceCount].center = positionWorldSpace;
    spheres[sphereMatriceCount].radius = radius;

    sphereMatrices[sphereMatriceCount] = translationMatrix * scaleMatrix;
    sphereMatriceCount++;
}

void DrawCommandBuilder::addLineToRender( const glm::vec3& from, const glm::vec3& to, const float thickness, const glm::vec4& color )
{
    worldRenderer->drawDebugLine( from, to, thickness, color );
}

void DrawCommandBuilder::addHUDRectangle( const glm::vec2& positionScreenSpace, const glm::vec2& dimensionsScreenSpace, const float rotationInRadians, const Material* material )
{
    auto mat1 = glm::translate( glm::mat4( 1.0f ), glm::vec3( positionScreenSpace, 0.0f ) );

    auto mat2 = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.5f * dimensionsScreenSpace.x, 0.5f * dimensionsScreenSpace.y, 0.0f ) );
    auto mat3 = glm::rotate( glm::mat4( 1.0f ), rotationInRadians, glm::vec3( 0.0f, 0.0f, 1.0f ) );
    auto mat4 = glm::translate( glm::mat4( 1.0f ), glm::vec3( -0.5f * dimensionsScreenSpace.x, -0.5f * dimensionsScreenSpace.y, 0.0f ) );

    auto mat5 = glm::scale( glm::mat4( 1.0f ),  glm::vec3( dimensionsScreenSpace, 1.0f ) );

    hudRectangle[hudRectangleMatriceCount].modelMatrix = mat1 * mat2 * mat3 * mat4 * mat5;
    hudRectangle[hudRectangleMatriceCount].material = material;

    hudRectangleMatriceCount++;
}

void DrawCommandBuilder::addHUDText( const std::string& text, const float scale, const float x, const float y, const float outlineThickness, const glm::vec4& color, const bool useNormalizedCoordinates )
{
    worldRenderer->drawDebugText( text, scale, x, y, outlineThickness, color, useNormalizedCoordinates );
}

void DrawCommandBuilder::buildCommands( RenderDevice* renderDevice, WorldRenderer* worldRenderer )
{
    int viewportIndex = 0;

    // Entities are independant from viewports; update them only once
    while ( !renderKeysToUpdate.empty() ) {
        auto renderKey = renderKeysToUpdate.front();
        renderableEntityManager->updateEntity( renderKey );
        renderKeysToUpdate.pop();
    }

    renderableEntityManager->rebuildBuffer( renderDevice );

    if ( envProbeCaptureCount > 0 ) {
        worldRenderer->drawDebugText( "[PROBE CAPTURE IN PROGRESS]", 0.3f, 1.0f, 0.2f, 0.0f, glm::vec4( 1, 0, 0, 1 ) );

        auto& probeToCapture = envProbeCaptureQueue[envProbeCaptureCount - 1 ];

FLAN_IMPORT_VAR_PTR( EnvProbeDimension, uint32_t )

        Viewport envProbeViewport = {};
        envProbeViewport.X = 0;
        envProbeViewport.Y = 0;
        envProbeViewport.Width = static_cast<int32_t>( *EnvProbeDimension );
        envProbeViewport.Height = static_cast<int32_t>( *EnvProbeDimension );
        envProbeViewport.MinDepth = 0.0f;
        envProbeViewport.MaxDepth = 1.0f;

        // Tweak probe field of view to avoid visible seams
        const float ENV_PROBE_FOV = 2.0f * atanf( *EnvProbeDimension / ( *EnvProbeDimension - 0.5f ) );
        constexpr float ENV_PROBE_ASPECT_RATIO = 1.0f;

        // Build probe face projection infos
        Camera::Data probeCamera = {};
        probeCamera.worldPosition = probeToCapture.Probe->Sphere.center;

        probeCamera.projectionMatrix = flan::core::MakeInfReversedZProj( ENV_PROBE_FOV, ENV_PROBE_ASPECT_RATIO, 0.01f );
        probeCamera.inverseProjectionMatrix = glm::transpose( glm::inverse( probeCamera.projectionMatrix ) );
        probeCamera.depthProjectionMatrix = glm::perspectiveFovLH( ENV_PROBE_FOV, (float)*EnvProbeDimension, ( float )*EnvProbeDimension, 1.0f, 125.0f );

        probeCamera.viewMatrix = GetProbeCaptureViewMatrix( probeToCapture.Probe->Sphere.center, probeToCapture.CommandInfos.Step );
        probeCamera.depthViewProjectionMatrix = probeCamera.depthProjectionMatrix * probeCamera.viewMatrix;
        probeCamera.inverseViewMatrix = glm::inverse( probeCamera.viewMatrix );

        probeCamera.viewProjectionMatrix = glm::transpose( probeCamera.projectionMatrix * probeCamera.viewMatrix );
        probeCamera.viewMatrix = glm::transpose( probeCamera.viewMatrix );

        probeCamera.inverseViewProjectionMatrix = glm::inverse( probeCamera.viewProjectionMatrix );

        // Build viewport
        auto& viewport = worldRenderer->addViewport( viewportIndex );
        viewport.worldViewport = probeCamera;
        viewport.rendererViewport = envProbeViewport;

        // Define Probe Capture Pipeline
        viewport.renderPasses.push_back( FLAN_STRING_HASH( "AtmosphereNoSunDiskRenderPass" ) );

        // Fallback probe should only capture atmosphere at frame N
        if ( !probeToCapture.Probe->IsFallbackProbe ) {
            viewport.renderPasses.push_back( FLAN_STRING_HASH( "CascadedShadowMapCapture" ) );
            viewport.renderPasses.push_back( FLAN_STRING_HASH( "WorldDepthPass" ) );
            viewport.renderPasses.push_back( FLAN_STRING_HASH( "LightCullingPass" ) );
            viewport.renderPasses.push_back( FLAN_STRING_HASH( "WorldLightProbePass" ) );

            // Create temporary frustum to cull geometry
            Frustum probeCameraFrustum;
            flan::core::UpdateFrustumPlanes( probeCamera.viewProjectionMatrix, probeCameraFrustum );

            addDepthGeometryForViewport( viewport.worldViewport, viewportIndex, DrawCommandKey::DEPTH_VIEWPORT_LAYER_DEFAULT, &probeCameraFrustum, worldRenderer );
            addGeometryForViewport( viewport.worldViewport, viewportIndex, DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT, &probeCameraFrustum, worldRenderer );
        }

        viewport.renderPasses.push_back( FLAN_STRING_HASH( "ProbeCaptureSavePass" ) );
        viewport.renderPassesArgs[FLAN_STRING_HASH( "ProbeCaptureSavePass" )] = &probeToCapture;

        viewportIndex++;
        envProbeCaptureCount--;
    }

    if ( envProbeConvolutionCount > 0 ) {
        worldRenderer->drawDebugText( "[PROBE CONVOLUTION IN PROGRESS]", 0.3f, 1.0f, 0.3f, 0.0f, glm::vec4( 1, 0, 0, 1 ) );
        auto& probeConvolutionCmd = envProbeConvolutionQueue[envProbeConvolutionCount - 1];

        FLAN_IMPORT_VAR_PTR( EnvProbeDimension, uint32_t )

        const auto mipLevelDimension = static_cast<int32_t>( *EnvProbeDimension >> probeConvolutionCmd.CommandInfos.MipIndex );

        Viewport envProbeViewport = {};
        envProbeViewport.X = 0;
        envProbeViewport.Y = 0;
        envProbeViewport.Width = mipLevelDimension;
        envProbeViewport.Height = mipLevelDimension;
        envProbeViewport.MinDepth = 0.0f;
        envProbeViewport.MaxDepth = 1.0f;

        // Build viewport
        auto& viewport = worldRenderer->addViewport( viewportIndex );
        viewport.rendererViewport = envProbeViewport;
        viewport.renderPasses.push_back( FLAN_STRING_HASH( "ProbeConvolutionPass" ) );
        viewport.renderPassesArgs[FLAN_STRING_HASH( "ProbeConvolutionPass" )] = &probeConvolutionCmd;

        viewportIndex++;
        envProbeConvolutionCount--;
    }

    // TODO It sucks.
        // Make this better (since CSM is viewport dependant, this is quite complicated to implement seamlessly...)
    auto directionalLight = renderableEntityManager->getDirectionalLightByIndex( 0 );
    for ( auto& camera : cameras ) {
        if ( directionalLight != nullptr
          && camera->isUsingRenderPass( FLAN_STRING_HASH( "CascadedShadowMapCapture" ) ) ) {
            // Update Camera CSM data
            auto& cameraData = camera->GetDataRW();          
            cameraData.globalShadowMatrix = flan::framework::CSMCreateGlobalShadowMatrix( directionalLight->getLightData().direction, cameraData.depthViewProjectionMatrix );

            for ( int sliceIdx = 0; sliceIdx < CSM_SLICE_COUNT; sliceIdx++ ) {
                flan::framework::CSMComputeSliceData( directionalLight, sliceIdx, cameraData );
            }

            cameraData.globalShadowMatrix = glm::transpose( cameraData.globalShadowMatrix );

            // Create temporary frustum to cull geometry
            Frustum csmCameraFrustum;
            for ( int sliceIdx = 0; sliceIdx < CSM_SLICE_COUNT; sliceIdx++ ) {
                flan::core::UpdateFrustumPlanes( cameraData.shadowViewMatrix[sliceIdx], csmCameraFrustum );

                addDepthGeometryForViewport( cameraData, viewportIndex, DrawCommandKey::DEPTH_VIEWPORT_LAYER_CSM0 + sliceIdx, &csmCameraFrustum, worldRenderer );
            }
        }
        
FLAN_IMPORT_VAR_PTR( WindowWidth, int32_t )
FLAN_IMPORT_VAR_PTR( WindowHeight, int32_t )
        Viewport viewportTest = {};
        viewportTest.X = 0;
        viewportTest.Y = 0;
        viewportTest.Width = *WindowWidth;
        viewportTest.Height = *WindowHeight;
        viewportTest.MinDepth = 0.0f;
        viewportTest.MaxDepth = 1.0f;

        // Build viewport
        auto& viewport = worldRenderer->addViewport( viewportIndex );
        viewport.worldViewport = camera->GetData();
        viewport.rendererViewport = viewportTest;
        viewport.renderPasses = std::vector<fnStringHash_t>( *camera->getRenderPassList() );

        // Add geometry to render
        addGeometryForViewport( viewport.worldViewport, viewportIndex, DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT, camera->getFrustum(), worldRenderer );
        addDepthGeometryForViewport( viewport.worldViewport, viewportIndex, DrawCommandKey::DEPTH_VIEWPORT_LAYER_DEFAULT, camera->getFrustum(), worldRenderer );
       
        if ( camera->isUsingRenderPass( FLAN_STRING_HASH( "DebugWorldPass" ) )
          || camera->isUsingRenderPass( FLAN_STRING_HASH( "DebugWorldMSAAPass" ) ) ) {
            addDebugGeometryForViewport( viewport.worldViewport, viewportIndex, DrawCommandKey::DEBUG_VIEWPORT_LAYER_DEFAULT, camera->getFrustum(), worldRenderer );
        }

        addHUDGeometryForViewport( viewport.worldViewport, viewportIndex, DrawCommandKey::HUD_VIEWPORT_LAYER_DEFAULT, worldRenderer );

        viewportIndex++;
    }

    cameras.clear();
    meshInstancesCount = 0;
    wireMeshInstancesCount = 0;
    sphereMatriceCount = 0;
    hudRectangleMatriceCount = 0;
}

void DrawCommandBuilder::addGeometryForViewport( const Camera::Data& worldViewport, const int viewportIndex, const uint8_t viewportLayer, const Frustum* viewportFrustum, WorldRenderer* worldRenderer )
{
    for ( int i = 0; i < meshInstancesCount; i++ ) {
        auto* meshInstance = meshInstances[i];
        auto& subMeshes = meshInstance->meshAsset->getSubMeshVector();

        auto meshVao = meshInstance->meshAsset->getVertexArrayObject();
        auto meshModelMatrix = meshInstance->meshTransform->getWorldModelMatrix();

        for ( auto& subMesh : subMeshes ) {
            BoundingSphere sphere = subMesh.boundingSphere;
            sphere.center += meshInstance->meshTransform->getWorldTranslation();
            sphere.radius *= meshInstance->meshTransform->getWorldBiggestScale();

            if ( CullSphereInfReversedZ( viewportFrustum, sphere.center, sphere.radius ) > 0 ) {
                auto distanceToCamera = glm::distance( sphere.center, worldViewport.worldPosition );

                DrawCommandKey drawCmdKey;
                drawCmdKey.bitfield.layer = DrawCommandKey::LAYER_WORLD;
                drawCmdKey.bitfield.viewportId = viewportIndex;
                drawCmdKey.bitfield.viewportLayer = viewportLayer;
                drawCmdKey.bitfield.sortOrder = ( subMesh.material->isOpaque() )
                        ? DrawCommandKey::SortOrder::SORT_FRONT_TO_BACK
                        : DrawCommandKey::SortOrder::SORT_BACK_TO_FRONT;
                drawCmdKey.bitfield.depth = DepthToBits( distanceToCamera );
                drawCmdKey.bitfield.materialSortKey = subMesh.material->getMaterialSortKey();

                DrawCommandInfos drawCmdGeo;
                drawCmdGeo.material = subMesh.material;
                drawCmdGeo.vao = meshVao;
                drawCmdGeo.indiceBufferOffset = subMesh.indiceBufferOffset;
                drawCmdGeo.indiceBufferCount = subMesh.indiceCount;
                drawCmdGeo.modelMatrix = meshModelMatrix;

                worldRenderer->addDrawCommand( { drawCmdKey, drawCmdGeo } );
            }
        }
    }
}

void DrawCommandBuilder::addDepthGeometryForViewport( const Camera::Data& worldViewport, const int viewportIndex, const uint8_t viewportLayer, const Frustum* viewportFrustum, WorldRenderer* worldRenderer )
{
    for ( int i = 0; i < meshInstancesCount; i++ ) {
        auto* meshInstance = meshInstances[i];
        auto& subMeshes = meshInstance->meshAsset->getSubMeshVector();

        auto meshVao = meshInstance->meshAsset->getVertexArrayObject();
        auto meshModelMatrix = meshInstance->meshTransform->getWorldModelMatrix();

        for ( auto& subMesh : subMeshes ) {
            BoundingSphere sphere = subMesh.boundingSphere;
            sphere.center += meshInstance->meshTransform->getWorldTranslation();
            sphere.radius *= meshInstance->meshTransform->getWorldBiggestScale();

            if ( CullSphereInfReversedZ( viewportFrustum, sphere.center, sphere.radius ) > 0 ) {
                auto distanceToCamera = glm::distance( sphere.center, worldViewport.worldPosition );

                DrawCommandKey drawCmdKey;
                drawCmdKey.bitfield.layer = DrawCommandKey::LAYER_DEPTH;
                drawCmdKey.bitfield.viewportId = viewportIndex;
                drawCmdKey.bitfield.viewportLayer = viewportLayer;
                drawCmdKey.bitfield.sortOrder = DrawCommandKey::SortOrder::SORT_FRONT_TO_BACK;
                drawCmdKey.bitfield.depth = DepthToBits( distanceToCamera );
                drawCmdKey.bitfield.materialSortKey = subMesh.material->getMaterialSortKey();

                DrawCommandInfos drawCmdGeo;
                drawCmdGeo.material = subMesh.material;
                drawCmdGeo.vao = meshVao;
                drawCmdGeo.indiceBufferOffset = subMesh.indiceBufferOffset;
                drawCmdGeo.indiceBufferCount = subMesh.indiceCount;
                drawCmdGeo.modelMatrix = meshModelMatrix;

                worldRenderer->addDrawCommand( { drawCmdKey, drawCmdGeo } );
            }
        }
    }
}

void DrawCommandBuilder::addDebugGeometryForViewport( const Camera::Data& worldViewport, const int viewportIndex, const uint8_t viewportLayer, const Frustum* viewportFrustum, WorldRenderer* worldRenderer )
{
    auto wireframeMat = worldRenderer->getWireframeMaterial();
    uint32_t sphereIndiceCount = 0;
    auto sphereVao = worldRenderer->getSpherePrimitive( sphereIndiceCount );

    for ( int debugPrimIdx = 0; debugPrimIdx < sphereMatriceCount; debugPrimIdx++ ) {
        if ( CullSphereInfReversedZ( viewportFrustum, spheres[debugPrimIdx].center, spheres[debugPrimIdx].radius ) > 0 ) {
            auto distanceToCamera = glm::distance( spheres[debugPrimIdx].center, worldViewport.worldPosition );
            DrawCommandKey drawCmdKey;
            drawCmdKey.bitfield.layer = DrawCommandKey::LAYER_DEBUG;
            drawCmdKey.bitfield.viewportId = viewportIndex;
            drawCmdKey.bitfield.viewportLayer = viewportLayer;
            drawCmdKey.bitfield.sortOrder = DrawCommandKey::SortOrder::SORT_BACK_TO_FRONT;
            drawCmdKey.bitfield.depth = DepthToBits( distanceToCamera );
            drawCmdKey.bitfield.materialSortKey = wireframeMat->getMaterialSortKey();

            DrawCommandInfos drawCmdGeo;
            drawCmdGeo.material = wireframeMat;
            drawCmdGeo.vao = sphereVao;
            drawCmdGeo.indiceBufferOffset = 0u;
            drawCmdGeo.indiceBufferCount = sphereIndiceCount;
            drawCmdGeo.modelMatrix = &sphereMatrices[debugPrimIdx];

            worldRenderer->addDrawCommand( { drawCmdKey, drawCmdGeo } );
        }
    }

    for ( int i = 0; i < wireMeshInstancesCount; i++ ) {
        auto* meshInstance = wireMeshInstances[i];
        auto& subMeshes = meshInstance->meshAsset->getSubMeshVector();

        auto meshVao = meshInstance->meshAsset->getVertexArrayObject();
        auto meshModelMatrix = meshInstance->meshTransform->getWorldModelMatrix();

        for ( auto& subMesh : subMeshes ) {
            BoundingSphere sphere = subMesh.boundingSphere;
            sphere.center += meshInstance->meshTransform->getWorldTranslation();
            sphere.radius *= meshInstance->meshTransform->getWorldBiggestScale();

            if ( CullSphereInfReversedZ( viewportFrustum, sphere.center, sphere.radius ) > 0 ) {
                auto distanceToCamera = glm::distance( sphere.center, worldViewport.worldPosition );

                DrawCommandKey drawCmdKey;
                drawCmdKey.bitfield.layer = DrawCommandKey::LAYER_DEBUG;
                drawCmdKey.bitfield.viewportId = viewportIndex;
                drawCmdKey.bitfield.viewportLayer = viewportLayer;
                drawCmdKey.bitfield.sortOrder = DrawCommandKey::SortOrder::SORT_BACK_TO_FRONT;
                drawCmdKey.bitfield.depth = DepthToBits( distanceToCamera );
                drawCmdKey.bitfield.materialSortKey = wireframeMat->getMaterialSortKey();

                DrawCommandInfos drawCmdGeo;
                drawCmdGeo.material = wireframeMat;
                drawCmdGeo.vao = meshVao;
                drawCmdGeo.indiceBufferOffset = subMesh.indiceBufferOffset;
                drawCmdGeo.indiceBufferCount = subMesh.indiceCount;
                drawCmdGeo.modelMatrix = meshModelMatrix;

                worldRenderer->addDrawCommand( { drawCmdKey, drawCmdGeo } );
            }
        }
    }
}

void DrawCommandBuilder::addHUDGeometryForViewport( const Camera::Data& worldViewport, const int viewportIndex, const uint8_t viewportLayer, WorldRenderer* worldRenderer )
{
    uint32_t rectIndiceCount = 0;
    auto rectangleVao = worldRenderer->getRectanglePrimitive( rectIndiceCount );

    for ( int rectPrimIdx = 0; rectPrimIdx < hudRectangleMatriceCount; rectPrimIdx++ ) {
        const auto material = hudRectangle[rectPrimIdx].material;

        DrawCommandKey drawCmdKey;
        drawCmdKey.bitfield.layer = DrawCommandKey::LAYER_HUD;
        drawCmdKey.bitfield.viewportId = viewportIndex;
        drawCmdKey.bitfield.viewportLayer = viewportLayer;
        drawCmdKey.bitfield.sortOrder = DrawCommandKey::SortOrder::SORT_BACK_TO_FRONT;
        drawCmdKey.bitfield.depth = DepthToBits( 0.0f );
        drawCmdKey.bitfield.materialSortKey = material->getMaterialSortKey();

        DrawCommandInfos drawCmdGeo;
        drawCmdGeo.material = material;
        drawCmdGeo.vao = rectangleVao;
        drawCmdGeo.indiceBufferOffset = 0u;
        drawCmdGeo.indiceBufferCount = rectIndiceCount;
        drawCmdGeo.modelMatrix = &hudRectangle[rectPrimIdx].modelMatrix;

        worldRenderer->addDrawCommand( { drawCmdKey, drawCmdGeo } );
    }
}

unsigned int DrawCommandBuilder::getFrameNumber() const
{
    return worldRenderer->getFrameNumber();
}
