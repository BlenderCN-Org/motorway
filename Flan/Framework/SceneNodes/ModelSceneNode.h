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

#include "SceneNode.h"

#include "Model.h"
#include <Io/TextStreamHelpers.h>

struct ModelSceneNode : public SceneNode 
{
private:
    void rebuildInstances()
    {
        if ( model != nullptr ) {
            for ( auto* mesh : model->meshes ) {
                instances.push_back( MeshInstance{ mesh, &transform } );
            }
        }
    }

public:
    static constexpr fnStringHash_t Hashcode = FLAN_STRING_HASH( "ModelSceneNode" );

    Model* model;
    std::vector<MeshInstance> instances;

    ModelSceneNode( Model* nodeModel, const std::string& name = "Model" )
        : SceneNode( name )
        , model( nodeModel )
    {
#if FLAN_DEVBUILD
        canCollectDebugRenderKeys = true;
#endif

        canCollectRenderKeys = true;
        canBeIntersected = true;

        rebuildInstances();
    }

    virtual void collectRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        if ( model != nullptr ) {
            for ( auto& instance : instances ) {
                if ( instance.meshAsset != nullptr ) {
                    drawCommandBuilder->addMeshToRender( &instance );
                }
            }
        }
    }


#if FLAN_DEVBUILD
    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime )
    {
        SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );

        auto modelPath = ( model != nullptr ) ? model->name : "(empty)";

        if ( ImGui::Button( "Add Mesh To Model" ) ) {
            if ( model == nullptr ) model = new Model();

            instances.push_back( { nullptr, &transform } );
        }

        ImGui::SameLine();
        if ( ImGui::Button( "Load Model" ) ) {
            fnString_t modelName;
            if ( flan::core::DisplayFileOpenPrompt( modelName, FLAN_STRING( "Model file (*.model)\0*.model" ), FLAN_STRING( "./" ), FLAN_STRING( "Select a Model" ) ) ) {
                modelName = fnString_t( modelName.c_str() );

                auto workingDir = fnString_t( FLAN_STRING( "" ) );
                flan::core::RetrieveWorkingDirectory( workingDir );

                workingDir.append( FLAN_STRING( "data" ) );
                size_t poswd = modelName.find( workingDir );

                if ( poswd != fnString_t::npos ) {
                    // If found then erase it from string
                    modelName.erase( poswd, workingDir.length() );
                }

                std::replace( modelName.begin(), modelName.end(), '\\', '/' );

                model = graphicsAssetManager->getModel( ( FLAN_STRING( "GameData" ) + modelName ).c_str() );

                rebuildInstances();
            }
        }

        for ( auto& instance : instances ) {
            auto meshPath = ( instance.meshAsset != nullptr ) ? flan::core::WideStringToString( instance.meshAsset->getName() ) : "(empty)";
            ImGui::LabelText( "##meshPath", meshPath.c_str() );
            ImGui::SameLine();

            if ( ImGui::Button( ( "...##" + meshPath ).c_str() ) ) {
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

            for ( auto& subMesh : instance.meshAsset->getSubMeshVectorRW() ) {
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

                        subMesh.material = graphicsAssetManager->getMaterialCopy( ( FLAN_STRING( "GameData" ) + materialName ).c_str() );
                    }
                }
                ImGui::SameLine();
                if ( ImGui::SmallButton( flan::core::WideStringToString( subMesh.material->getName() ).c_str() ) ) {
                    FLAN_IMPORT_VAR_PTR( dev_EditorPickedMaterial, Material* );
                    *dev_EditorPickedMaterial = subMesh.material;
                }
            }
        }
    }

    virtual void collectDebugRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        if ( DrawBoundingPrimitive ) {
            for ( auto& instance : instances ) {
                for ( auto& subMesh : instance.meshAsset->getSubMeshVector() ) {
                    auto subMeshBoundingSphere = subMesh.boundingSphere;
                    subMeshBoundingSphere.center += instance.meshTransform->getWorldTranslation();
                    subMeshBoundingSphere.radius *= instance.meshTransform->getWorldBiggestScale();

                    drawCommandBuilder->addWireframeSphere( subMeshBoundingSphere.center, subMeshBoundingSphere.radius );
                }

                FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* );
                if ( *PickedNode == this ) {
                    if ( instance.meshAsset == nullptr ) {
                        continue;
                    }

                    drawCommandBuilder->addWireframeMeshToRender( &instance );
                }
            }
        }
    }
#endif

    virtual void serialize( FileSystemObject* stream, const fnStringHash_t sceneNodeHashcode ) override
    {
        SceneNode::serialize( stream, Hashcode );

        if ( !model->name.empty() )
            stream->writeString( model->name );

        stream->write<uint8_t>( 0x0 );
    }

    virtual void deserialize( FileSystemObject* stream, GraphicsAssetManager* graphicsAssetManager, RenderableEntityManager* renderableEntityManager )  override
    {
        SceneNode::deserialize( stream, graphicsAssetManager, renderableEntityManager );

        fnString_t modelName;
        flan::core::ReadString( stream, modelName );

        if ( !modelName.empty() )
            model = graphicsAssetManager->getModel( modelName.c_str() );

        rebuildInstances();
    }

    virtual bool intersect( const Ray& ray, float& hitDistance ) const override
    {
        if ( model == nullptr ) {
            return false;
        }

        for ( auto& instance : instances ) {
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
            if ( flan::core::RayAABBIntersectionTest( boundingBox, ray, hitDistance, dontCare ) ) {
                return true;
            }
        }

        return false;
    }
};
