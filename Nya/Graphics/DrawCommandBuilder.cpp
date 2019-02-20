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

#include "WorldRenderer.h"
#include "RenderPipeline.h"
#include "LightGrid.h"

#include <Rendering/RenderDevice.h>

#include "RenderModules/BrunetonSkyRenderModule.h"
#include "RenderModules/AutomaticExposureRenderModule.h"
#include "RenderPasses/PresentRenderPass.h"
#include "RenderPasses/LightRenderPass.h"
#include "RenderPasses/CopyRenderPass.h"
#include "RenderPasses/FinalPostFxRenderPass.h"
#include "RenderPasses/BlurPyramidRenderPass.h"
#include "RenderPasses/MSAAResolveRenderPass.h"

#include <Maths/Helpers.h>
#include <Maths/Matrix.h>

class TextRenderingModule
{
public:
    MutableResHandle_t renderText( RenderPipeline* renderPipeline, MutableResHandle_t output );
};

float DistanceToPlane( const nyaVec4f& vPlane, const nyaVec3f& vPoint )
{
    return nyaVec4f::dot( nyaVec4f( vPoint, 1.0f ), vPlane );
}

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
    : cameraCount( 0 )
    , meshCount( 0 )
{
    cameras = nya::core::allocateArray<const CameraData*>( allocator, 8, nullptr );
    meshes = nya::core::allocateArray<MeshInstance>( allocator, 4096 );
}

DrawCommandBuilder::~DrawCommandBuilder()
{

}

void DrawCommandBuilder::addGeometryToRender( const Mesh* meshResource, const nyaMat4x4f* modelMatrix )
{
    meshes[meshCount++] = { meshResource, modelMatrix };
}

void DrawCommandBuilder::addCamera( const CameraData* cameraData )
{
    cameras[cameraCount++] = cameraData;
}

void DrawCommandBuilder::buildRenderQueues( WorldRenderer* worldRenderer, LightGrid* lightGrid )
{
    NYA_PROFILE_FUNCTION

    for ( uint32_t cameraIdx = 0; cameraIdx < cameraCount; cameraIdx++ ) {
        const CameraData* camera = cameras[cameraIdx];

        // Register viewport into the world renderer
        RenderPipeline& renderPipeline = worldRenderer->allocateRenderPipeline( { 0, 0,  static_cast<int32_t>( camera->viewportSize.x ), static_cast<int32_t>( camera->viewportSize.y ), 0.0f, 1.0f }, camera );
        renderPipeline.setMSAAQuality( 8 );

        bool useTAA = true;

        // !!TEST!!
        renderPipeline.beginPassGroup();
        {
            auto skyRenderTarget = worldRenderer->skyRenderModule->renderSky( &renderPipeline );
            auto lightRenderTarget = AddLightRenderPass( &renderPipeline, lightGrid->getLightsClusters(), lightGrid->getLightsBuffer(), lightGrid->getClustersInfos(), skyRenderTarget );

            auto resolvedTarget = AddMSAAResolveRenderPass( &renderPipeline, lightRenderTarget.lightRenderTarget, lightRenderTarget.velocityRenderTarget, lightRenderTarget.depthRenderTarget, 8, useTAA );

            if ( useTAA ) {
                AddCurrentFrameSaveRenderPass( &renderPipeline, resolvedTarget );
            }

            worldRenderer->automaticExposureModule->computeExposure( &renderPipeline, resolvedTarget, camera->viewportSize );

            auto blurPyramid = AddBlurPyramidRenderPass( &renderPipeline, resolvedTarget, static_cast<uint32_t>( camera->viewportSize.x ), static_cast<uint32_t>( camera->viewportSize.y ) );

            // NOTE UI Rendering should be done in linear space! (once async compute is implemented, UI rendering will be parallelized with PostFx)
            auto hudRenderTarget = worldRenderer->textRenderModule->renderText( &renderPipeline, resolvedTarget );

            auto postFxRenderTarget = AddFinalPostFxRenderPass( &renderPipeline, resolvedTarget, blurPyramid );
            AddPresentRenderPass( &renderPipeline, postFxRenderTarget );
        }

        // Cull static mesh instances
        for ( uint32_t meshIdx = 0; meshIdx < meshCount; meshIdx++ ) {
            const MeshInstance& meshInstance = meshes[meshIdx];

            const nyaVec3f instancePosition = ( *meshInstance.modelMatrix )[3];
            const float distanceToCamera = nyaVec3f::distanceSquared( camera->worldPosition, instancePosition );

            // Retrieve LOD based on instance to camera distance
            const auto& activeLOD = meshInstance.mesh->getLevelOfDetail( distanceToCamera );

            const Buffer* vertexBuffer = meshInstance.mesh->getVertexBuffer();
            const Buffer* indiceBuffer = meshInstance.mesh->getIndiceBuffer();

            for ( const SubMesh& subMesh : activeLOD.subMeshes ) {
                // Transform sphere origin by instance model matrix
                nyaVec3f position = instancePosition + subMesh.boundingSphere.center;

                if ( CullSphereInfReversedZ( &camera->frustum, position, subMesh.boundingSphere.radius ) > 0.0f ) {
                    // Build drawcmd is the submesh is visible
                    DrawCmd& drawCmd = worldRenderer->allocateDrawCmd();

                    auto& key = drawCmd.key.bitfield;
                    key.materialSortKey = subMesh.material->getSortKey();
                    key.depth = DepthToBits( distanceToCamera );
                    key.sortOrder = ( subMesh.material->isOpaque() ) ? DrawCommandKey::SORT_FRONT_TO_BACK : DrawCommandKey::SORT_BACK_TO_FRONT;
                    key.layer = DrawCommandKey::LAYER_WORLD;
                    key.viewportLayer = DrawCommandKey::WORLD_VIEWPORT_LAYER_DEFAULT;
                    key.viewportId = static_cast< uint8_t >( cameraIdx );

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
    }

    resetEntityCounters();
}

void DrawCommandBuilder::resetEntityCounters()
{
    cameraCount = 0;
    meshCount = 0;
}
