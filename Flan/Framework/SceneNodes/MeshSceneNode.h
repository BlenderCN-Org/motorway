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

#include "Mesh.h"
#include "SceneNode.h"
#include <Graphics/DrawCommandBuilder.h>
#include <Io/TextStreamHelpers.h>
#include <Core/LocaleHelpers.h>
#include <Core/Environment.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Framework/Material.h>

#include <Core/FileSystemIOHelpers.h>

#include <Core/LocaleHelpers.h>

FLAN_DEV_VAR( DrawBoundingPrimitive, "Draw Bounding Primitive for geometry (wireframe) [false/true]", false, bool )

struct MeshSceneNode : public SceneNode
{
    virtual MeshSceneNode* clone( RenderableEntityManager* renderableEntityManager )
    {
        return new MeshSceneNode( *this );
    }

    static constexpr fnStringHash_t Hashcode = FLAN_STRING_HASH( "MeshSceneNode" );

    MeshInstance instance;

    MeshSceneNode( Mesh* nodeMesh, const std::string& name = "Mesh" )
        : SceneNode( name )
        , instance()
    {
        instance.meshAsset = nodeMesh;
        instance.meshTransform = &transform;

        canCollectRenderKeys = true;

#if FLAN_DEVBUILD
        canCollectDebugRenderKeys = true;
#endif

        canBeIntersected = true;
    }

    MeshSceneNode( MeshSceneNode& node )
        : SceneNode( node )
    {
        instance.meshAsset = node.instance.meshAsset;
        instance.meshTransform = &transform;
    }

    virtual void collectRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        if ( instance.meshAsset != nullptr ) {
            drawCommandBuilder->addMeshToRender( &instance );
        }
    }

#if FLAN_DEVBUILD
    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime )
    {
        SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );
        
        auto meshPath = ( instance.meshAsset != nullptr ) ? flan::core::WideStringToString( instance.meshAsset->getName() ) : "(empty)";
        ImGui::LabelText( "##meshPath", meshPath.c_str() );
        ImGui::SameLine();

        if ( ImGui::Button( "..." ) ) {
            fnString_t meshName;
            if ( flan::core::DisplayFileOpenPrompt( meshName, FLAN_STRING( "Mesh file (*.mesh)\0*.mesh" ), FLAN_STRING( "./" ), FLAN_STRING( "Select a Mesh" ) ) ) {
                meshName = fnString_t( meshName.c_str() );

                auto workingDir = fnString_t( FLAN_STRING( "" ) );
                flan::core::RetrieveWorkingDirectory( workingDir );

                workingDir.append( FLAN_STRING( "data" ) );
                size_t poswd = meshName.find( workingDir );

                if ( poswd != fnString_t::npos ) {
                    // If found then erase it from string
                    meshName.erase( poswd, workingDir.length() );
                }

                std::replace( meshName.begin(), meshName.end(), '\\', '/' );

                instance.meshAsset = graphicsAssetManager->getMesh( ( FLAN_STRING( "GameData" ) + meshName ).c_str() );
            }
        }

        if ( instance.meshAsset == nullptr ) {
            return;
        }
        
        for ( int lodIdx = 0; lodIdx < Mesh::MAX_LOD_COUNT; lodIdx++ ) {
            const auto& lod = instance.meshAsset->getLevelOfDetailByIndex( lodIdx );

            if ( lod.startDistance < 0.0f ) {
                continue;
            }

            if ( ImGui::TreeNode( std::string( "LOD" + std::to_string( lodIdx ) ).c_str() ) ) {
                ImGui::LabelText( "##loddistance", std::string( "Distance: " + std::to_string( lod.lodDistance ) ).c_str() );

                for ( auto& subMesh : lod.subMeshes ) {
                    auto str = flan::core::WideStringToString( subMesh.name );
                    ImGui::LabelText( ( "##" + str ).c_str(), str.c_str() );

                    ImGui::SameLine();

                    if ( ImGui::SmallButton( ( "...##" + str ).c_str() ) ) {
                        fnString_t materialName;
                        if ( flan::core::DisplayFileOpenPrompt( materialName, FLAN_STRING( "Material Asset file (*.amat)\0*.amat" ), FLAN_STRING( "./" ), FLAN_STRING( "Select a Material Asset" ) ) ) {
                            materialName = fnString_t( materialName.c_str() );

                            auto workingDir = fnString_t( FLAN_STRING( "" ) );
                            flan::core::RetrieveWorkingDirectory( workingDir );
                            workingDir.append( FLAN_STRING( "data" ) );

                            size_t poswd = materialName.find( workingDir );

                            if ( poswd != fnString_t::npos ) {
                                // If found then erase it from string
                                materialName.erase( poswd, workingDir.length() );
                            }

                            std::replace( materialName.begin(), materialName.end(), '\\', '/' );

                            //subMesh.material = graphicsAssetManager->getMaterialCopy( ( FLAN_STRING( "GameData" ) + materialName ).c_str() );
                        }
                    }

                    ImGui::SameLine();

                    if ( ImGui::SmallButton( ( flan::core::WideStringToString( subMesh.material->getName() ) + "##" + str ).c_str() ) ) {
                        FLAN_IMPORT_VAR_PTR( dev_EditorPickedMaterial, Material* );
                        *dev_EditorPickedMaterial = subMesh.material;
                    }
                }

                ImGui::TreePop();
            }
        }
    }

    virtual void collectDebugRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        if ( DrawBoundingPrimitive ) {
            auto subMeshBoundingSphere = instance.meshAsset->getBoundingSphere(); // subMesh.boundingSphere;
            subMeshBoundingSphere.center += instance.meshTransform->getWorldTranslation();
            subMeshBoundingSphere.radius *= instance.meshTransform->getWorldBiggestScale();

            drawCommandBuilder->addWireframeSphere( subMeshBoundingSphere.center, subMeshBoundingSphere.radius );
        }

        FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* );
        if ( *PickedNode == this ) {
            if ( instance.meshAsset == nullptr ) {
                return;
            }

            drawCommandBuilder->addWireframeMeshToRender( &instance );
        }
    }
#endif

    virtual void serialize( FileSystemObject* stream, const fnStringHash_t sceneNodeHashcode ) override
    {
        SceneNode::serialize( stream, Hashcode );

        auto name = flan::core::WideStringToString( instance.meshAsset->getName() );
        stream->writeString( name );
        stream->write<uint8_t>( 0x0 );
    }

    virtual void deserialize( FileSystemObject* stream, GraphicsAssetManager* graphicsAssetManager, RenderableEntityManager* renderableEntityManager )  override
    {
        SceneNode::deserialize( stream, graphicsAssetManager, renderableEntityManager );

        fnString_t meshName;
        flan::core::ReadString( stream, meshName );

        instance.meshAsset = graphicsAssetManager->getMesh( meshName.c_str() );
        instance.meshTransform = &transform;
    }

    virtual bool intersect( const Ray& ray, float& hitDistance ) const override
    {
        if ( instance.meshAsset == nullptr ) {
            return false;
        }

        auto nodeScale = transform.getWorldScale();
        auto nodeTranslation = transform.getWorldTranslation();

        auto& meshAABB = instance.meshAsset->getAABB();

        AABB boundingBox;
        flan::core::CreateAABBFromMinMaxPoints( 
            boundingBox, 
            ( nodeTranslation + meshAABB.minPoint * nodeScale ),
            ( nodeTranslation + meshAABB.maxPoint * nodeScale )
        );

        float dontCare = 0.0f;
        return flan::core::RayAABBIntersectionTest( boundingBox, ray, hitDistance, dontCare );
    }
};
