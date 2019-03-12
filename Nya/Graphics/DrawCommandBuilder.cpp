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

#include "Shared.h"
#include "DrawCommandBuilder.h"

#include <Framework/Cameras/Camera.h>
#include <Framework/Mesh.h>
#include <Framework/Material.h>
#include <Framework/DirectionalLightHelpers.h>
#include <Framework/Light.h>

#include "WorldRenderer.h"
#include "RenderPipeline.h"
#include "LightGrid.h"

#include <Rendering/RenderDevice.h>

#include "RenderModules/BrunetonSkyRenderModule.h"
#include "RenderModules/AutomaticExposureRenderModule.h"
#include "RenderModules/ProbeCaptureModule.h"
#include "RenderModules/TextRenderingModule.h"
#include "RenderPasses/PresentRenderPass.h"
#include "RenderPasses/LightRenderPass.h"
#include "RenderPasses/CopyRenderPass.h"
#include "RenderPasses/FinalPostFxRenderPass.h"
#include "RenderPasses/BlurPyramidRenderPass.h"
#include "RenderPasses/MSAAResolveRenderPass.h"
#include "RenderPasses/CascadedShadowMapCapturePass.h"

#include <Maths/Helpers.h>
#include <Maths/Matrix.h>

#include <Shaders/Shared.h>

#include <Core/EnvVarsRegister.h>
#include <Core/Allocators/StackAllocator.h>
#include <Core/Allocators/PoolAllocator.h>

NYA_ENV_VAR( DisplayDebugIBLProbe, true, bool )

nyaMat4x4f GetProbeCaptureViewMatrix( const nyaVec3f& probePositionWorldSpace, const eProbeCaptureStep captureStep )
{
    switch ( captureStep ) {
    case eProbeCaptureStep::FACE_X_PLUS:
        return nya::maths::MakeLookAtMat(
            probePositionWorldSpace,
            probePositionWorldSpace + nyaVec3f( 1.0f, 0.0f, 0.0f ),
            nyaVec3f( 0.0f, 1.0f, 0.0f ) );

    case eProbeCaptureStep::FACE_X_MINUS:
        return nya::maths::MakeLookAtMat(
            probePositionWorldSpace,
            probePositionWorldSpace + nyaVec3f( -1.0f, 0.0f, 0.0f ),
            nyaVec3f( 0.0f, 1.0f, 0.0f ) );

    case eProbeCaptureStep::FACE_Y_MINUS:
        return nya::maths::MakeLookAtMat(
            probePositionWorldSpace,
            probePositionWorldSpace + nyaVec3f( 0.0f, -1.0f, 0.0f ),
            nyaVec3f( 0.0f, 0.0f, 1.0f ) );

    case eProbeCaptureStep::FACE_Y_PLUS:
        return nya::maths::MakeLookAtMat(
            probePositionWorldSpace,
            probePositionWorldSpace + nyaVec3f( 0.0f, 1.0f, 0.0f ),
            nyaVec3f( 0.0f, 0.0f, -1.0f ) );

    case eProbeCaptureStep::FACE_Z_PLUS:
        return nya::maths::MakeLookAtMat(
            probePositionWorldSpace,
            probePositionWorldSpace + nyaVec3f( 0.0f, 0.0f, 1.0f ),
            nyaVec3f( 0.0f, 1.0f, 0.0f ) );

    case eProbeCaptureStep::FACE_Z_MINUS:
        return nya::maths::MakeLookAtMat(
            probePositionWorldSpace,
            probePositionWorldSpace + nyaVec3f( 0.0f, 0.0f, -1.0f ),
            nyaVec3f( 0.0f, 1.0f, 0.0f ) );

    default:
        return nyaMat4x4f::Identity;
    }

    return nyaMat4x4f::Identity;
}

float DistanceToPlane( const nyaVec4f& vPlane, const nyaVec3f& vPoint )
{
    return nyaVec4f::dot( nyaVec4f( vPoint, 1.0f ), vPlane );
}

unsigned int FloatFlip( unsigned int f )
{
    unsigned mask = -int( f >> 31 ) | 0x80000000;
    return f ^ mask;
}

// Taking highest 10 bits for rough sort of floats.
// 0.01 maps to 752; 0.1 to 759; 1.0 to 766; 10.0 to 772;
// 100.0 to 779 etc. Negative numbers go similarly in 0..511 range.
uint16_t DepthToBits( const float depth )
{
    union { float f; unsigned int i; } f2i;
    f2i.f = depth;
    f2i.i = FloatFlip( f2i.i ); // flip bits to be sortable
    unsigned int b = f2i.i >> 22; // take highest 10 bits
    return static_cast<uint16_t>( b );
}

// Frustum cullling on a sphere. Returns > 0 if visible, <= 0 otherwise
// NOTE Infinite Z version (it implicitly skips the far plane check)
float CullSphereInfReversedZ( const Frustum* frustum, const nyaVec3f& vCenter, float fRadius )
{
    float dist01 = nya::maths::min( DistanceToPlane( frustum->planes[0], vCenter ), DistanceToPlane( frustum->planes[1], vCenter ) );
    float dist23 = nya::maths::min( DistanceToPlane( frustum->planes[2], vCenter ), DistanceToPlane( frustum->planes[3], vCenter ) );
    float dist45 = DistanceToPlane( frustum->planes[5], vCenter );

    return nya::maths::min( nya::maths::min( dist01, dist23 ), dist45 ) + fRadius;
}

DrawCommandBuilder::DrawCommandBuilder( BaseAllocator* allocator )
    : memoryAllocator( allocator )
{
    cameras = nya::core::allocate<PoolAllocator>( allocator, sizeof( CameraData* ), 4, 8 * sizeof( CameraData* ), allocator->allocate( 8 * sizeof( CameraData* ) ) );
    meshes = nya::core::allocate<PoolAllocator>( allocator, sizeof( MeshInstance ), 4, 4096 * sizeof( MeshInstance ), allocator->allocate( 4096 * sizeof( MeshInstance ) ) );
    spheresToRender = nya::core::allocate<PoolAllocator>( allocator, sizeof( nyaMat4x4f ), 4, 4096 * sizeof( nyaMat4x4f ), allocator->allocate( 4096 * sizeof( nyaMat4x4f ) ) );

    probeCaptureCmdAllocator = nya::core::allocate<StackAllocator>( allocator, 16 * 6 * sizeof( IBLProbeCaptureCommand ), allocator->allocate( 16 * 6 * sizeof( IBLProbeCaptureCommand ) ) );
    probeConvolutionCmdAllocator = nya::core::allocate<StackAllocator>( allocator, 16 * 8 * 6 * sizeof( IBLProbeConvolutionCommand ), allocator->allocate( 16 * 8 * 6 * sizeof( IBLProbeConvolutionCommand ) ) );
}

DrawCommandBuilder::~DrawCommandBuilder()
{
    nya::core::free( memoryAllocator, cameras );
    nya::core::free( memoryAllocator, meshes );
    nya::core::free( memoryAllocator, probeCaptureCmdAllocator );
    nya::core::free( memoryAllocator, probeConvolutionCmdAllocator );
}

void DrawCommandBuilder::addGeometryToRender( const Mesh* meshResource, const nyaMat4x4f* modelMatrix )
{
    auto* mesh = nya::core::allocate<MeshInstance>( meshes );
    mesh->mesh = meshResource;
    mesh->modelMatrix = modelMatrix;
}

void DrawCommandBuilder::addSphereToRender( const nyaVec3f& sphereCenter, const float sphereRadius )
{
    auto sphereMatrix = nya::core::allocate<nyaMat4x4f>( spheresToRender );
    *sphereMatrix = nya::maths::MakeTranslationMat( sphereCenter ) * nya::maths::MakeScaleMat( sphereRadius );
}

void DrawCommandBuilder::addCamera( CameraData* cameraData )
{
    auto camera = nya::core::allocate<CameraData*>( cameras );
    *camera = cameraData;
}

void DrawCommandBuilder::addIBLProbeToCapture( const IBLProbeData* probeData )
{
    for ( uint32_t i = 0; i < 6; i++ ) {
        auto* probeCaptureCmd = nya::core::allocate<IBLProbeCaptureCommand>( probeCaptureCmdAllocator );
        probeCaptureCmd->CommandInfos.EnvProbeArrayIndex = probeData->ProbeIndex;
        probeCaptureCmd->CommandInfos.Step = static_cast<eProbeCaptureStep>( i );
        probeCaptureCmd->Probe = probeData;

        probeCaptureCmds.push( probeCaptureCmd );

        for ( uint16_t mipIndex = 0; mipIndex < 8; mipIndex++ ) {
            auto* probeConvolutionCmd = nya::core::allocate<IBLProbeConvolutionCommand>( probeConvolutionCmdAllocator );
            probeConvolutionCmd->CommandInfos.EnvProbeArrayIndex = probeData->ProbeIndex;
            probeConvolutionCmd->CommandInfos.Step = static_cast<eProbeCaptureStep>( i );
            probeConvolutionCmd->CommandInfos.MipIndex = mipIndex;
            probeConvolutionCmd->Probe = probeData;

            probeConvolutionCmds.push( probeConvolutionCmd );
        }
    }
}

void DrawCommandBuilder::buildRenderQueues( WorldRenderer* worldRenderer, LightGrid* lightGrid )
{
    NYA_PROFILE_FUNCTION

    uint32_t cameraIdx = 0;
    CameraData** cameraArray = static_cast<CameraData**>( cameras->getBaseAddress() );
    const size_t cameraCount = cameras->getAllocationCount();

    for ( ; cameraIdx < cameraCount; cameraIdx++ ) {
        CameraData* camera = cameraArray[cameraIdx];

        // Register viewport into the world renderer
        RenderPipeline& renderPipeline = worldRenderer->allocateRenderPipeline( { 0, 0, static_cast<int32_t>( camera->viewportSize.x ), static_cast<int32_t>( camera->viewportSize.y ), 0.0f, 1.0f }, camera );
        renderPipeline.setMSAAQuality( camera->msaaSamplerCount );
        renderPipeline.setImageQuality( camera->imageQuality );

        worldRenderer->probeCaptureModule->importResourcesToPipeline( &renderPipeline );

        renderPipeline.beginPassGroup();
        {
            auto lightClustersData = lightGrid->updateClusters( &renderPipeline );

            auto sunShadowMap = AddCSMCapturePass( &renderPipeline );

            auto skyRenderTarget = worldRenderer->SkyRenderModule->renderSky( &renderPipeline );
            auto lightRenderTarget = AddLightRenderPass( &renderPipeline, lightClustersData, sunShadowMap, skyRenderTarget );

            auto resolvedTarget = AddMSAAResolveRenderPass( &renderPipeline, lightRenderTarget.lightRenderTarget, lightRenderTarget.velocityRenderTarget, lightRenderTarget.depthRenderTarget, camera->msaaSamplerCount, camera->flags.enableTAA );
            AddCurrentFrameSaveRenderPass( &renderPipeline, resolvedTarget );

            if ( camera->imageQuality != 1.0f ) {
                resolvedTarget = AddCopyAndDownsampleRenderPass( &renderPipeline, resolvedTarget, static_cast< uint32_t >( camera->viewportSize.x ), static_cast< uint32_t >( camera->viewportSize.y ) );
            }

            worldRenderer->automaticExposureModule->computeExposure( &renderPipeline, resolvedTarget, camera->viewportSize );

            auto blurPyramid = AddBlurPyramidRenderPass( &renderPipeline, resolvedTarget, static_cast<uint32_t>( camera->viewportSize.x ), static_cast<uint32_t>( camera->viewportSize.y ) );

            // NOTE UI Rendering should be done in linear space! (once async compute is implemented, UI rendering will be parallelized with PostFx)
            auto hudRenderTarget = worldRenderer->TextRenderModule->renderText( &renderPipeline, resolvedTarget );

            auto postFxRenderTarget = AddFinalPostFxRenderPass( &renderPipeline, resolvedTarget, blurPyramid );
            AddPresentRenderPass( &renderPipeline, postFxRenderTarget );
        }

        // CSM Capture
        // TODO Check if we can skip CSM capture depending on sun orientation?
        const DirectionalLightData* sunLight = lightGrid->getDirectionalLightData();

        camera->globalShadowMatrix = nya::framework::CSMCreateGlobalShadowMatrix( sunLight->direction, camera->depthViewProjectionMatrix );

        for ( int sliceIdx = 0; sliceIdx < CSM_SLICE_COUNT; sliceIdx++ ) {
            nya::framework::CSMComputeSliceData( sunLight, sliceIdx, camera );
        }

        camera->globalShadowMatrix = camera->globalShadowMatrix.transpose();

        // Create temporary frustum to cull geometry
        Frustum csmCameraFrustum;
        for ( int sliceIdx = 0; sliceIdx < CSM_SLICE_COUNT; sliceIdx++ ) {
            nya::maths::UpdateFrustumPlanes( camera->shadowViewMatrix[sliceIdx], csmCameraFrustum );

            // Cull static mesh instances (depth viewport)
            buildMeshDrawCmds( worldRenderer, camera, static_cast< uint8_t >( cameraIdx ), DrawCommandKey::LAYER_DEPTH, static_cast<DrawCommandKey::WorldViewportLayer>( DrawCommandKey::DEPTH_VIEWPORT_LAYER_CSM0 + sliceIdx ) );
        }

        // Cull static mesh instances (world viewport)
        buildMeshDrawCmds( worldRenderer, camera, static_cast< uint8_t >( cameraIdx ), DrawCommandKey::LAYER_WORLD, DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT );
    }

    if ( !probeCaptureCmds.empty() ) {
        IBLProbeCaptureCommand* cmd = probeCaptureCmds.top();

        // Tweak probe field of view to avoid visible seams
        const float ENV_PROBE_FOV = 2.0f * atanf( IBL_PROBE_DIMENSION / ( IBL_PROBE_DIMENSION - 0.5f ) );
        constexpr float ENV_PROBE_ASPECT_RATIO = 1.0f;

        CameraData probeCamera = {};
        probeCamera.worldPosition = cmd->Probe->worldPosition;

        probeCamera.projectionMatrix = nya::maths::MakeInfReversedZProj( ENV_PROBE_FOV, ENV_PROBE_ASPECT_RATIO, 0.01f );
        probeCamera.inverseProjectionMatrix = probeCamera.projectionMatrix.inverse();
        probeCamera.depthProjectionMatrix = nya::maths::MakeFovProj( ENV_PROBE_FOV, 1.0f, 1.0f, 125.0f );

        probeCamera.viewMatrix = GetProbeCaptureViewMatrix( cmd->Probe->worldPosition, cmd->CommandInfos.Step );
        probeCamera.depthViewProjectionMatrix = probeCamera.depthProjectionMatrix * probeCamera.viewMatrix;

        probeCamera.inverseViewMatrix = probeCamera.viewMatrix.inverse();

        probeCamera.viewProjectionMatrix = probeCamera.projectionMatrix * probeCamera.viewMatrix;
        probeCamera.inverseViewProjectionMatrix = probeCamera.viewProjectionMatrix.inverse();

        probeCamera.viewportSize = { IBL_PROBE_DIMENSION, IBL_PROBE_DIMENSION };
        probeCamera.imageQuality = 1.0f;
        probeCamera.msaaSamplerCount = 1;
        
        RenderPipeline& renderPipeline = worldRenderer->allocateRenderPipeline( { 0, 0, IBL_PROBE_DIMENSION, IBL_PROBE_DIMENSION, 0.0f, 1.0f }, &probeCamera );
        worldRenderer->probeCaptureModule->importResourcesToPipeline( &renderPipeline );

        if ( !cmd->Probe->isFallbackProbe ) {
            nya::maths::UpdateFrustumPlanes( probeCamera.depthViewProjectionMatrix, probeCamera.frustum );

            // CSM Capture
            // TODO Check if we can skip CSM capture depending on sun orientation?
            const DirectionalLightData* sunLight = lightGrid->getDirectionalLightData();

            probeCamera.globalShadowMatrix = nya::framework::CSMCreateGlobalShadowMatrix( sunLight->direction, probeCamera.depthViewProjectionMatrix );

            for ( int sliceIdx = 0; sliceIdx < CSM_SLICE_COUNT; sliceIdx++ ) {
                nya::framework::CSMComputeSliceData( sunLight, sliceIdx, &probeCamera );
            }

            probeCamera.globalShadowMatrix = probeCamera.globalShadowMatrix.transpose();

            // Create temporary frustum to cull geometry
            Frustum csmCameraFrustum;
            for ( int sliceIdx = 0; sliceIdx < CSM_SLICE_COUNT; sliceIdx++ ) {
                nya::maths::UpdateFrustumPlanes( probeCamera.shadowViewMatrix[sliceIdx], csmCameraFrustum );

                // Cull static mesh instances (depth viewport)
                buildMeshDrawCmds( worldRenderer, &probeCamera, static_cast< uint8_t >( cameraIdx ), DrawCommandKey::LAYER_DEPTH, static_cast<DrawCommandKey::WorldViewportLayer>( DrawCommandKey::DEPTH_VIEWPORT_LAYER_CSM0 + sliceIdx ) );
            }

            buildMeshDrawCmds( worldRenderer, &probeCamera, static_cast< uint8_t >( cameraIdx ), DrawCommandKey::LAYER_WORLD, DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT );
        }

        renderPipeline.beginPassGroup();
        {
            auto faceRenderTarget = worldRenderer->SkyRenderModule->renderSky( &renderPipeline, false, false );

            // Capture World
            if ( !cmd->Probe->isFallbackProbe ) {
                auto lightClustersData = lightGrid->updateClusters( &renderPipeline );

                auto sunShadowMap = AddCSMCapturePass( &renderPipeline );

                auto lightRenderTarget = AddLightRenderPass( &renderPipeline, lightClustersData, sunShadowMap, faceRenderTarget, true );

                faceRenderTarget = lightRenderTarget.lightRenderTarget;
            }

            worldRenderer->probeCaptureModule->saveCapturedProbeFace( &renderPipeline, faceRenderTarget, cmd->CommandInfos.EnvProbeArrayIndex, cmd->CommandInfos.Step );
        }

        cameraIdx++;

        probeCaptureCmdAllocator->free( cmd );
        probeCaptureCmds.pop();
    } else if ( !probeConvolutionCmds.empty() ) {
        IBLProbeConvolutionCommand* cmd = probeConvolutionCmds.top();

        RenderPipeline& renderPipeline = worldRenderer->allocateRenderPipeline( { 0, 0, IBL_PROBE_DIMENSION, IBL_PROBE_DIMENSION, 0.0f, 1.0f }, nullptr );
        worldRenderer->probeCaptureModule->importResourcesToPipeline( &renderPipeline );
        
        renderPipeline.beginPassGroup();
        {
            worldRenderer->probeCaptureModule->convoluteProbeFace( &renderPipeline, cmd->CommandInfos.EnvProbeArrayIndex, cmd->CommandInfos.Step, cmd->CommandInfos.MipIndex );
        }

        cameraIdx++;

        probeConvolutionCmdAllocator->free( cmd );
        probeConvolutionCmds.pop();
    }

    resetEntityCounters();
}

void DrawCommandBuilder::resetEntityCounters()
{
    cameras->clear();
    meshes->clear();
    spheresToRender->clear();
}

void DrawCommandBuilder::buildMeshDrawCmds( WorldRenderer* worldRenderer, CameraData* camera, const uint8_t cameraIdx, const uint8_t layer, const uint8_t viewportLayer )
{
    // TEST TEST TEST
    Material* shitTest = nullptr;

    MeshInstance* meshesArray = static_cast<MeshInstance*>( meshes->getBaseAddress() );
    const size_t meshCount = meshes->getAllocationCount();
    for ( uint32_t meshIdx = 0; meshIdx < meshCount; meshIdx++ ) {
        const MeshInstance& meshInstance = meshesArray[meshIdx];

        const nyaVec3f instancePosition = nya::maths::ExtractTranslation( *meshInstance.modelMatrix );

        const float instanceScale = nya::maths::GetBiggestScalar( nya::maths::ExtractScale( *meshInstance.modelMatrix ) );

        const float distanceToCamera = nyaVec3f::distanceSquared( camera->worldPosition, instancePosition );

        // Retrieve LOD based on instance to camera distance
        const auto& activeLOD = meshInstance.mesh->getLevelOfDetail( distanceToCamera );

        const Buffer* vertexBuffer = meshInstance.mesh->getVertexBuffer();
        const Buffer* indiceBuffer = meshInstance.mesh->getIndiceBuffer();

        for ( const SubMesh& subMesh : activeLOD.subMeshes ) {
            // Transform sphere origin by instance model matrix
            nyaVec3f position = instancePosition + subMesh.boundingSphere.center;
            float scaledRadius = instanceScale * subMesh.boundingSphere.radius;

            if ( CullSphereInfReversedZ( &camera->frustum, position, scaledRadius ) > 0.0f ) {
                // Build drawcmd is the submesh is visible
                DrawCmd& drawCmd = worldRenderer->allocateDrawCmd();

                auto& key = drawCmd.key.bitfield;
                key.materialSortKey = subMesh.material->getSortKey();
                key.depth = DepthToBits( distanceToCamera );
                key.sortOrder = ( subMesh.material->isOpaque() ) ? DrawCommandKey::SORT_FRONT_TO_BACK : DrawCommandKey::SORT_BACK_TO_FRONT;
                key.layer = static_cast<DrawCommandKey::Layer>( layer );
                key.viewportLayer = viewportLayer;
                key.viewportId = cameraIdx;

                // TEST TEST TEST
                shitTest = subMesh.material;

                DrawCommandInfos& infos = drawCmd.infos;
                infos.material = subMesh.material;
                infos.vertexBuffer = vertexBuffer;
                infos.indiceBuffer = indiceBuffer;
                infos.indiceBufferOffset = subMesh.indiceBufferOffset;
                infos.indiceBufferCount = subMesh.indiceCount;
                infos.alphaDitheringValue = 1.0f;
                infos.instanceCount = 1;
                infos.modelMatrix = meshInstance.modelMatrix;
            }
        }
    }

    // TEST TEST TEST
    if ( shitTest == nullptr ) return;

    nyaMat4x4f* sphereToRenderArray = static_cast<nyaMat4x4f*>( spheresToRender->getBaseAddress() );
    const size_t sphereCount = spheresToRender->getAllocationCount();
    for ( uint32_t sphereIdx = 0; sphereIdx < sphereCount; sphereIdx++ ) {
        sphereToRender[sphereIdx] = sphereToRenderArray[sphereIdx];

        DrawCmd& drawCmd = worldRenderer->allocateSpherePrimitiveDrawCmd();
        drawCmd.infos.material = shitTest;
        drawCmd.infos.instanceCount = 1;
        drawCmd.infos.modelMatrix = &sphereToRender[sphereIdx];

        auto& key = drawCmd.key.bitfield;
        key.materialSortKey = shitTest->getSortKey();
        key.depth = DepthToBits( 0.0f );
        key.sortOrder = DrawCommandKey::SORT_BACK_TO_FRONT;
        key.layer = static_cast<DrawCommandKey::Layer>( layer );
        key.viewportLayer = viewportLayer;
        key.viewportId = cameraIdx;
    }
}
