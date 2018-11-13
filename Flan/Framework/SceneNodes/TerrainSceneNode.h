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

#include <Framework/Terrain.h>
#include "SceneNode.h"
#include <Graphics/DrawCommandBuilder.h>
#include <Io/TextStreamHelpers.h>
#include <Core/LocaleHelpers.h>
#include <Core/Environment.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Framework/Material.h>

#include <Core/FileSystemIOHelpers.h>

#include <Core/LocaleHelpers.h>

struct TerrainSceneNode : public SceneNode
{
    virtual TerrainSceneNode* clone( RenderableEntityManager* renderableEntityManager )
    {
        return new TerrainSceneNode( *this );
    }

    static constexpr fnStringHash_t Hashcode = FLAN_STRING_HASH( "TerrainSceneNode" );

    TerrainInstance instance;
    MeshInstance grassInstane[512];
    TerrainSceneNode( Terrain* nodeTerrain, const std::string& name = "Terrain" )
        : SceneNode( name )
        , instance()
    {
        instance.terrainAsset = nodeTerrain;
        instance.meshTransform = &transform;

        for ( int i = 0; i < 512; i++ ) {
            grassInstane[i].meshAsset = nodeTerrain->GRASS_TEST;
            grassInstane[i].meshTransform = &nodeTerrain->grassTestTransform[i];
        }

        canCollectRenderKeys = true;

#if FLAN_DEVBUILD
        canCollectDebugRenderKeys = true;
#endif

        canBeIntersected = true;
    }

    TerrainSceneNode( TerrainSceneNode& node )
        : SceneNode( node )
    {
        instance.terrainAsset = node.instance.terrainAsset;
        instance.meshTransform = &transform;
    }

    virtual void collectRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        if ( instance.terrainAsset != nullptr ) {
            drawCommandBuilder->addTerrainToRender( &instance );

            drawCommandBuilder->addMeshesToRender( grassInstane, 512 );
        }
    }

#if FLAN_DEVBUILD
    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime )
    {
        SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );

        ImGui::LabelText( "##terrainNode", "Terrain" );

        if ( instance.terrainAsset != nullptr ) {
            if ( ImGui::SmallButton( ( flan::core::WideStringToString( instance.terrainAsset->getMaterial()->getName() ) ).c_str() ) ) {
                FLAN_IMPORT_VAR_PTR( dev_EditorPickedMaterial, Material* );
                *dev_EditorPickedMaterial = instance.terrainAsset->getMaterial();
            }
        }
    }

    virtual void collectDebugRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        if ( DrawBoundingPrimitive ) {
            const auto& boundingBox = instance.terrainAsset->getAxisAlignedBoundingBox();
            auto center = boundingBox.minPoint + boundingBox.maxPoint;
            auto extent = boundingBox.maxPoint - boundingBox.minPoint;

            drawCommandBuilder->addWireframeAABB( center, extent );
        }
    }
#endif

    virtual void serialize( FileSystemObject* stream, const fnStringHash_t sceneNodeHashcode ) override
    {
        SceneNode::serialize( stream, Hashcode );
    }

    virtual void deserialize( FileSystemObject* stream, GraphicsAssetManager* graphicsAssetManager, RenderableEntityManager* renderableEntityManager )  override
    {
        SceneNode::deserialize( stream, graphicsAssetManager, renderableEntityManager );
    }

    virtual bool intersect( const Ray& ray, float& hitDistance ) const override
    {
        const auto& boundingBox = instance.terrainAsset->getAxisAlignedBoundingBox();
        float dontCare = 0;

        return flan::core::RayAABBIntersectionTest( boundingBox, ray, hitDistance, dontCare );
    }
};
