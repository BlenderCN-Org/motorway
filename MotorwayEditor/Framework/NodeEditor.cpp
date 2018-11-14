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
#include "NodeEditor.h"

#include <AppShared.h>

#include <Framework/SceneNodes/SceneNode.h>
#include <Framework/TransactionHandler/TransactionHandler.h>
#include <Framework/TransactionHandler/SceneNodeCopyCommand.h>
#include <Framework/TransactionHandler/SceneNodeDeleteCommand.h>

#include <FileSystem/FileSystemObjectNative.h>

#include <Io/Collider.h>

#include <imgui/imgui.h>

FLAN_DEV_VAR( dev_IsInputText, "Is Using a Text Input", false, bool )

// ImGui Getter for vector
bool VectorOfStringGetter( void* data, int n, const char** out_text )
{
    static constexpr char* SCENE_ROOT_NAME = "Scene Root";

    const std::vector<SceneNode*>* v = ( std::vector<SceneNode*>* )data;

    if ( n == 0 ) {
        *out_text = SCENE_ROOT_NAME;
    } else {
        *out_text = ( *v )[n - 1]->name.c_str();
    }

    return true;
}

static void RebuildRigidBody( RigidBody* rigidBody )
{
    // 'It just werks'!
    g_DynamicsWorld->removeRigidBody( rigidBody );
    rigidBody->recomputeInertia();
    g_DynamicsWorld->addRigidBody( rigidBody );
}

void flan::framework::DisplayNodeEditor( const float deltaTime )
{
    FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* )
        if ( *PickedNode != nullptr ) {
            auto node = *PickedNode;

            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.96f, 0.1f, 0.05f, 1.0f ) );
            if ( ImGui::Button( "Delete!" ) ) {
                ImGui::PopStyleColor();

                g_TransactionHandler->commit( new SceneNodeDeleteCommand( node, g_CurrentScene, g_RenderableEntityManager, g_DynamicsWorld ) );
                *PickedNode = nullptr;
            } else {
                ImGui::PopStyleColor();

                ImGui::Separator();

                bool isDynamic = ( node->rigidBody != nullptr );
                if ( ImGui::Checkbox( "Is Dynamic", &isDynamic ) ) {
                    if ( isDynamic ) {
                        node->rigidBody = new RigidBody( 0.0f, node->transform.getWorldTranslation(), node->transform.getWorldRotation() );

                        g_DynamicsWorld->addRigidBody( node->rigidBody );
                    } else {
                        g_DynamicsWorld->removeRigidBody( node->rigidBody );
                        delete node->rigidBody;
                        node->rigidBody = nullptr;
                    }
                }

                const auto& sceneNodes = g_CurrentScene->getSceneNodes();

                int selectedParentIdx = 0;
                if ( node->parent != nullptr ) {
                    for ( int sceneNodeIdx = 0; sceneNodeIdx < sceneNodes.size(); sceneNodeIdx++ ) {
                        if ( node->parent == sceneNodes[sceneNodeIdx] ) {
                            selectedParentIdx = sceneNodeIdx + 1;
                            break;
                        }
                    }
                }

                if ( ImGui::Combo( "Parent", &selectedParentIdx, VectorOfStringGetter, ( void* )&sceneNodes, ( int )sceneNodes.size() ) ) {
                    if ( node->parent != nullptr ) {
                        node->parent->children.erase( std::find( node->parent->children.begin(), node->parent->children.end(), node ) );
                    }

                    if ( selectedParentIdx == 0 ) {
                        node->parent = nullptr;
                    } else {
                        auto selectedParent = sceneNodes[selectedParentIdx - 1];
                        node->parent = selectedParent;
                        selectedParent->children.push_back( node );
                    }
                }

                if ( node->rigidBody != nullptr ) {
                    ImGui::Separator();
                    ImGui::LabelText( "##RigidBodyName", "RigidBody" );

                    auto nativeObject = node->rigidBody->getNativeObject();
                    auto activationState = nativeObject->getActivationState();
                    auto shape = nativeObject->getCollisionShape();
                    int shapeType = shape->getShapeType();

                    enum eRigidBodyShape : int
                    {
                        UNSUPPORTED = 0,

                        BOX,
                        SPHERE,
                        CAPSULE,
                        CYLINDER,
                        CONE,
                        STATIC_PLANE,
                        CONVEX_HULL,
                        HEIGHTFIELD,

                        COUNT
                    };

                    static eRigidBodyShape btToShape[29] = {
                        BOX,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        CONVEX_HULL,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        SPHERE,
                        UNSUPPORTED,
                        CAPSULE,
                        UNSUPPORTED, //CONE,
                        UNSUPPORTED,
                        CYLINDER,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        HEIGHTFIELD,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        UNSUPPORTED,
                        STATIC_PLANE,
                    };

                    static constexpr char* COLLISION_SHAPES_LABELS[eRigidBodyShape::COUNT] = {
                        "(empty)", "Box", "Sphere", "Capsule", "Cylinder", "Cone", "Static Plane", "Convex Hull", "Heightmap"
                    };

                    int genericShapeType = btToShape[shapeType];
                    if ( ImGui::Combo( "Collision Shape", &genericShapeType, COLLISION_SHAPES_LABELS, eRigidBodyShape::COUNT ) ) {
                        switch ( genericShapeType ) {
                        case eRigidBodyShape::BOX:
                            nativeObject->setCollisionShape( new btBoxShape( btVector3( 1, 1, 1 ) ) );
                            RebuildRigidBody( node->rigidBody );
                            break;
                        case eRigidBodyShape::SPHERE:
                            nativeObject->setCollisionShape( new btSphereShape( 1 ) );
                            RebuildRigidBody( node->rigidBody );
                            break;
                        case eRigidBodyShape::CAPSULE:
                            nativeObject->setCollisionShape( new btCapsuleShape( 1, 1 ) );
                            RebuildRigidBody( node->rigidBody );
                            break;
                        case eRigidBodyShape::CYLINDER:
                            nativeObject->setCollisionShape( new btCylinderShape( btVector3( 1, 1, 1 ) ) );
                            RebuildRigidBody( node->rigidBody );
                            break;
                        case eRigidBodyShape::STATIC_PLANE:
                            nativeObject->setCollisionShape( new btStaticPlaneShape( btVector3( 0, 1, 0 ), 1 ) );
                            RebuildRigidBody( node->rigidBody );
                            break;
                        case eRigidBodyShape::CONVEX_HULL:
                        {
                            fnString_t convexHullFile;
                            if ( flan::core::DisplayFileOpenPrompt( convexHullFile, FLAN_STRING( "Mesh Convex Hull file (*.mesh.hull)\0*.mesh.hull" ), FLAN_STRING( "./" ), FLAN_STRING( "Select a Mesh Convex Hull" ) ) ) {
                                auto file = new FileSystemObjectNative( convexHullFile );
                                file->open( std::ios::binary | std::ios::in );

                                std::vector<float> vertices;
                                Io_ReadConvexHullColliderFile( file, vertices );
                                file->close();
                                delete file;

                                auto shape = new btConvexHullShape();
                                for ( int i = 0; i < vertices.size(); i += 3 ) {
                                    shape->addPoint( btVector3( vertices[i], vertices[i + 1], vertices[i + 2] ), false );
                                }
                                shape->recalcLocalAabb();

                                nativeObject->setCollisionShape( shape );

                                RebuildRigidBody( node->rigidBody );
                            }
                        } break;
                        case eRigidBodyShape::HEIGHTFIELD:
                        {
                            /*       fnString_t hmapFile;
                            if ( flan::core::DisplayFileOpenPrompt( hmapFile, FLAN_STRING( "Heightmap file (*.hmap)\0*.hmap" ), FLAN_STRING( "./" ), FLAN_STRING( "Select Heightmap file" ) ) ) {
                            fnString_t assetPath;
                            flan::core::ExtractFilenameFromPath( hmapFile, assetPath );

                            std::size_t hmapPrecision;
                            void* hmap = g_GraphicsAssetManager->getImageTexels( ( FLAN_STRING( "GameData/Textures/" ) + assetPath ).c_str(), hmapPrecision );

                            std::vector<float> texels( 512 * 512 );
                            float minH = std::numeric_limits<float>::max(), maxH = -std::numeric_limits<float>::max();
                            for ( int texelId = 0; texelId < 512 * 512; texelId++ ) {
                            float height = ( ( uint16_t* )hmap )[texelId] / std::numeric_limits<float>::max();

                            minH = std::min( minH, height );
                            maxH = std::max( maxH, height );

                            texels[texelId] = height;
                            }

                            auto shape = new btHeightfieldTerrainShape( 512, 512, texels.data(), 1.0f, minH, maxH, 1, PHY_FLOAT, false );

                            nativeObject->setCollisionShape( shape );

                            RebuildRigidBody( node->rigidBody );
                            } */
                        } break;

                        default:
                            break;
                        }
                    }

                    switch ( genericShapeType ) {
                    case eRigidBodyShape::CONVEX_HULL:
                    {
                        auto hullShape = static_cast< btConvexHullShape* >( nativeObject->getCollisionShape() );

                        if ( ImGui::Button( "..." ) ) {
                            fnString_t convexHullFile;
                            if ( flan::core::DisplayFileOpenPrompt( convexHullFile, FLAN_STRING( "Mesh Convex Hull file (*.mesh.hull)\0*.mesh.hull" ), FLAN_STRING( "./" ), FLAN_STRING( "Select a Mesh Convex Hull" ) ) ) {
                                auto file = new FileSystemObjectNative( convexHullFile );
                                file->open( std::ios::binary | std::ios::in );

                                std::vector<float> vertices;
                                Io_ReadConvexHullColliderFile( file, vertices );
                                file->close();
                                delete file;

                                auto shape = new btConvexHullShape();
                                for ( int i = 0; i < vertices.size(); i += 3 ) {
                                    shape->addPoint( btVector3( vertices[i], vertices[i + 1], vertices[i + 2] ), false );
                                }
                                shape->recalcLocalAabb();

                                nativeObject->setCollisionShape( shape );

                                RebuildRigidBody( node->rigidBody );
                            }
                        }

                        auto vertexCount = hullShape->getNumVertices();
                        ImGui::LabelText( "Vertex Count", "%i", vertexCount );

                        if ( ImGui::Button( "Optimize Geometry" ) ) {
                            hullShape->optimizeConvexHull();
                        }
                    } break;

                    case eRigidBodyShape::BOX:
                    {
                        auto boxShape = static_cast< btBoxShape* >( nativeObject->getCollisionShape() );
                        auto boxHalfSize = boxShape->getHalfExtentsWithoutMargin();

                        if ( ImGui::DragFloat3( "Collider Half Dimensions", &boxHalfSize[0] ) ) {
                            boxShape->setImplicitShapeDimensions( boxHalfSize );
                            RebuildRigidBody( node->rigidBody );
                        }
                    } break;

                    case eRigidBodyShape::SPHERE:
                    {
                        auto boxShape = static_cast< btSphereShape* >( nativeObject->getCollisionShape() );
                        auto sphereRadius = boxShape->getRadius();

                        if ( ImGui::DragFloat( "Collider Radius", &sphereRadius ) ) {
                            boxShape->setUnscaledRadius( sphereRadius );
                            RebuildRigidBody( node->rigidBody );
                        }
                    } break;

                    case eRigidBodyShape::CAPSULE:
                    {
                        auto capsuleShape = static_cast< btCapsuleShape* >( nativeObject->getCollisionShape() );
                        auto capsuleHalfHeight = capsuleShape->getHalfHeight();
                        auto capsuleWidth = capsuleShape->getRadius();

                        btVector3 capsuleDimension( capsuleWidth, capsuleHalfHeight, 0 );
                        if ( ImGui::DragFloat3( "Collider Half Dimensions", &capsuleDimension[0] ) ) {
                            capsuleShape->setImplicitShapeDimensions( capsuleDimension );
                            RebuildRigidBody( node->rigidBody );
                        }
                    } break;

                    case eRigidBodyShape::STATIC_PLANE:
                    {
                        auto planeShape = static_cast< btStaticPlaneShape* >( nativeObject->getCollisionShape() );
                        auto planeHeight = planeShape->getPlaneConstant();
                        auto planeNormal = planeShape->getPlaneNormal();

                        // TODO Don't leak memory like a turbo retard!
                        if ( ImGui::DragFloat3( "Plane Normal", &planeNormal[0] ) ) {
                            nativeObject->setCollisionShape( new btStaticPlaneShape( planeNormal, planeHeight ) );
                            RebuildRigidBody( node->rigidBody );
                        }

                        if ( ImGui::DragFloat( "Plane Height", &planeHeight ) ) {
                            nativeObject->setCollisionShape( new btStaticPlaneShape( planeNormal, planeHeight ) );
                            RebuildRigidBody( node->rigidBody );
                        }
                    } break;

                    case eRigidBodyShape::HEIGHTFIELD:
                    {
                        auto hmapShape = static_cast< btHeightfieldTerrainShape* >( nativeObject->getCollisionShape() );
                    } break;


                    default:
                        break;
                    };

                    static constexpr char* ACTIVATION_STATE_LABELS[6] = { "Disabled", "Enabled", "Island Sleeping", "Disabled", "Keep Alive", "Wake Up" };
                    if ( ImGui::Combo( "Activation State", &activationState, ACTIVATION_STATE_LABELS, 6 ) ) {
                        nativeObject->setActivationState( activationState );
                    }

                    auto gravity = nativeObject->getGravity();
                    if ( ImGui::DragFloat3( "Gravity", &gravity[0] ) ) {
                        nativeObject->setGravity( gravity );
                    }

                    auto rigidBodyMass = node->rigidBody->getMass();
                    if ( ImGui::DragFloat( "Mass", &rigidBodyMass ) ) {
                        node->rigidBody->setMass( rigidBodyMass );
                        node->rigidBody->recomputeInertia();
                        RebuildRigidBody( node->rigidBody );
                    }

                    auto friction = nativeObject->getFriction();
                    if ( ImGui::DragFloat( "Friction", &friction ) ) {
                        nativeObject->setFriction( friction );
                    }

                    auto restitution = nativeObject->getRestitution();
                    if ( ImGui::DragFloat( "Restitution", &restitution ) ) {
                        nativeObject->setRestitution( restitution );
                    }

                    ImGui::Separator();
                }

                // Draw Node Edition Panel Content
                node->drawInEditor( g_GraphicsAssetManager, g_TransactionHandler, deltaTime );
            }

        } else {
            dev_IsInputText = false;
        }
}
