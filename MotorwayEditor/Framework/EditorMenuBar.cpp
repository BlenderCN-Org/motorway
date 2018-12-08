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
#include "EditorMenuBar.h"

#include <AppShared.h>

#include <Framework/Scene.h>
#include <Core/Allocators/LinearAllocator.h>

#include <Rendering/CommandList.h>
#include <Graphics/RenderableEntityManager.h>
#include <Graphics/RenderPasses/CompositionPass.h>

#include <Io/Scene.h>
#include <Physics/DynamicsWorld.h>

#include <FileSystem/FileSystemObjectNative.h>

#include <Framework/TransactionHandler/TransactionHandler.h>
#include <Framework/TransactionHandler/SceneNodeCopyCommand.h>
#include <Framework/TransactionHandler/SceneNodeDeleteCommand.h>

#include <imgui/imgui.h>

struct SceneNode;

void flan::framework::DisplayEditorMenuBar()
{
    FLAN_IMPORT_VAR_PTR( DrawBoundingPrimitive, bool )
    FLAN_IMPORT_VAR_PTR( EnableTemporalAA, bool )
    FLAN_IMPORT_VAR_PTR( PrintViewportInfos, bool )
    FLAN_IMPORT_VAR_PTR( EnableCPUProfilerPrint, bool )
    FLAN_IMPORT_VAR_PTR( EnablePipelineProfiling, bool )
    FLAN_IMPORT_VAR_PTR( EnableCPUFPSPrint, bool )
    FLAN_IMPORT_VAR_PTR( EnableVSync, bool )
    FLAN_IMPORT_VAR_PTR( EnableDebugPhysicsColliders, bool )
    FLAN_IMPORT_VAR_PTR( EnableFXAA, bool )
    FLAN_IMPORT_VAR_PTR( DisplayTileHeat, bool )
    FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* )
    FLAN_IMPORT_VAR_PTR( CopiedNode, SceneNode* )
    FLAN_IMPORT_VAR_PTR( GrassLODDebugView, bool )
    FLAN_IMPORT_VAR_PTR( PrintMemoryDebugInfos, bool )
        
    FLAN_IMPORT_VAR_PTR( GraphicsCompositionSettings, CompositionSettings )

    if ( ImGui::BeginMainMenuBar() ) {
        if ( ImGui::BeginMenu( "File" ) ) {
            if ( ImGui::MenuItem( "New Scene" ) ) {
                *PickedNode = nullptr;
                g_RenderableEntityManager->clear();
                g_CurrentScene = ( new Scene() );

                auto camera = flan::core::allocate<FreeCamera>( g_GlobalAllocator );
                camera->SetProjectionMatrix( 80.0f, 1920.0f, 1080.0f );

                auto sceneNode = ( FreeCameraSceneNode* )g_CurrentScene->createFreeCamera( camera, "DefaultCamera" );
                sceneNode->enabled = true;
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "Load Scene..." ) ) {
                fnString_t sceneName;
                if ( flan::core::DisplayFileOpenPrompt( sceneName, FLAN_STRING( "Asset Scene file (*.scene)\0*.scene" ), FLAN_STRING( "./" ), FLAN_STRING( "Save as a Scene asset" ) ) ) {
                    auto file = new FileSystemObjectNative( sceneName );
                    file->open( std::ios::binary | std::ios::in );

                    *PickedNode = nullptr;
                    g_RenderableEntityManager->clear();

                    Scene* loadedScene = new Scene();

                    // Trigger scene change (flush CPU/GPU buffers; discard current game state; etc.)
                    g_CurrentScene = loadedScene;

                    // Then load the scene
                    Io_ReadSceneFile( file, g_GraphicsAssetManager, g_RenderableEntityManager, *loadedScene );
                    file->close();
                    delete file;

                    for ( auto& node : g_CurrentScene->getSceneNodes() ) {
                        if ( node->rigidBody != nullptr ) {
                            g_DynamicsWorld->addRigidBody( node->rigidBody );
                        }
                    }

                    // Find active editor camera (dont use the gameplay one in editor mode!)
                    auto cameraNode = static_cast< FreeCameraSceneNode* >( loadedScene->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) ) );
                    if ( cameraNode != nullptr ) {
                        cameraNode->enabled = true;
                    } else {
                        FLAN_CWARN << "No camera defined/loaded; exepect garbage and weird stuff on screen..." << std::endl;
                    }

                    //    paString_t fileNameWithoutExtension, sceneNameWithExt;
                    //    pa::core::ExtractFilenameFromPath( sceneName, fileNameWithoutExtension );
                    //    pa::core::GetFilenameWithoutExtension( fileNameWithoutExtension, sceneNameWithExt );

                    //    g_RenderManager->LoadProbesFromDisk( g_GraphicsAssetManager, paString_t( sceneNameWithExt.c_str() ) );

                    //    // Rebuild BVH
                }
            }

            if ( ImGui::MenuItem( "Save Scene..." ) ) {
                fnString_t sceneName;
                if ( flan::core::DisplayFileSavePrompt( sceneName, FLAN_STRING( "Asset Scene file (*.scene)\0*.scene" ), FLAN_STRING( "./" ), FLAN_STRING( "Save as a Scene asset" ) ) ) {
                    sceneName = fnString_t( sceneName.c_str() );
                    auto file = new FileSystemObjectNative( fnString_t( sceneName + FLAN_STRING( ".scene" ) ) );
                    file->open( std::ios::binary | std::ios::out );
                    Io_WriteSceneFile( g_CurrentScene, file );
                    file->close();
                    delete file;

                    //paString_t fileNameWithoutExtension, sceneNameWithExt;
                    //pa::core::ExtractFilenameFromPath( sceneName, fileNameWithoutExtension );
                    //pa::core::GetFilenameWithoutExtension( fileNameWithoutExtension, sceneNameWithExt );

                    //// Save Probes to disk
                    //g_RenderManager->SaveProbesToDisk( paString_t( sceneNameWithExt.c_str() ) );
                }
            }

            if ( ImGui::MenuItem( "Export Scene..." ) ) {

            }

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( "Edit" ) ) {
            if ( ImGui::MenuItem( "Undo" ) ) {
                g_TransactionHandler->undo();
            }

            if ( ImGui::MenuItem( "Redo" ) ) {
                g_TransactionHandler->redo();
            }

            ImGui::Separator();

            if ( ImGui::MenuItem( "Copy" ) ) {
                if ( PickedNode != nullptr ) {
                    *CopiedNode = *PickedNode;
                }
            }

            if ( ImGui::MenuItem( "Paste" ) ) {
                if ( CopiedNode != nullptr ) {
                    g_TransactionHandler->commit<SceneNodeCopyCommand>( *PickedNode, g_CurrentScene, g_RenderableEntityManager, g_DynamicsWorld );
                }
            }

            if ( ImGui::MenuItem( "Delete" ) ) {
                if ( PickedNode != nullptr ) {
                    g_TransactionHandler->commit<SceneNodeDeleteCommand>( *PickedNode, g_CurrentScene, g_RenderableEntityManager, g_DynamicsWorld );
                    PickedNode = nullptr;
                }
            }
            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( "Add To Scene" ) ) {
            auto scene = g_CurrentScene;

            if ( ImGui::MenuItem( "Free Camera" ) ) {
                auto camera = new FreeCamera();
                camera->SetProjectionMatrix( 80.0f, 16.0f, 9.0f );
                auto sceneNode = scene->createFreeCamera( camera );
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "First Person Camera" ) ) {
                auto camera = new FirstPersonCamera();
                camera->setProjectionMatrix( 80.0f, 16.0f, 9.0f );
                auto sceneNode = scene->createFirstPersonCamera( camera );
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "Look at Camera" ) ) {
                auto camera = new LookAtCamera();
                camera->setProjectionMatrix( 80.0f, 16.0f, 9.0f );
                auto sceneNode = scene->createLookAtCamera( camera );
                *PickedNode = sceneNode;
            }
            ImGui::Separator();

            if ( ImGui::MenuItem( "Mesh" ) ) {
                auto sceneNode = scene->createMesh( nullptr );
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "Model" ) ) {
                auto sceneNode = scene->createModel( nullptr );
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "Terrain" ) ) {
                auto terrain = flan::core::allocate<Terrain>( g_GlobalAllocator );

                GraphicsAssetManager::RawTexels hmapTexels;
                g_GraphicsAssetManager->getImageTexels( FLAN_STRING( "GameData/Textures/heightmap_test.hmap" ), hmapTexels );

                GraphicsAssetManager::RawTexels splatMapTexels;
                g_GraphicsAssetManager->getImageTexels( FLAN_STRING( "GameData/Textures/splatmap.png16" ), splatMapTexels );

                GraphicsAssetManager::RawTexels grassMapTexels;
                g_GraphicsAssetManager->getImageTexels( FLAN_STRING( "GameData/Textures/grassmap_test.png16" ), grassMapTexels );

                terrain->create( g_RenderDevice,
                    g_GlobalAllocator,
                    g_GraphicsAssetManager->getMaterialCopy( FLAN_STRING( "GameData/Materials/DefaultTerrainMaterial.amat" ) ),
                    ( uint16_t* )grassMapTexels.data,
                    ( uint16_t* )splatMapTexels.data,
                    ( uint16_t* )hmapTexels.data,
                    hmapTexels.width, 
                    hmapTexels.height );

                auto sceneNode = scene->createTerrain( terrain );
                *PickedNode = sceneNode;

                float minH = terrain->getHeightmapLowestVertex();
                float maxH = terrain->getHeightmapHighestVertex();

                auto shape = new btHeightfieldTerrainShape( hmapTexels.width, hmapTexels.height, terrain->getHeightmapValuesHeightScaled(), 1.0f, minH, maxH, 1, PHY_FLOAT, false );

                float offset = ( minH + maxH ) * 0.5f;
                sceneNode->rigidBody = new RigidBody( 0.0f, glm::vec3( 256, offset, 256 ), sceneNode->transform.getWorldRotation() );

                auto nativeObject = sceneNode->rigidBody->getNativeObject();
                nativeObject->setCollisionShape( shape );

                sceneNode->rigidBody->recomputeInertia();
                g_DynamicsWorld->addRigidBody( sceneNode->rigidBody );
            }

            ImGui::Separator();

            if ( ImGui::MenuItem( "Directional Light" ) ) {
                auto dirLight = g_RenderableEntityManager->createDirectionalLight( DirectionalLight() );
                auto sceneNode = scene->createDirectionalLight( dirLight );
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "Point Light" ) ) {
                auto pointLight = g_RenderableEntityManager->createPointLight( PointLightData{ glm::vec3( 0, 0.5, 0 ), 1.0f } );

                auto sceneNode = scene->createPointLight( pointLight );
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "Spot Light" ) ) {
                auto spotLight = g_RenderableEntityManager->createSpotLight( SpotLightData{ glm::vec3( 0, 0.5, 0 ), 1.0f } );

                auto sceneNode = scene->createSpotLight( spotLight );
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "Sphere Light" ) ) {
                auto sphereLight = g_RenderableEntityManager->createSphereLight( PointLightData{ glm::vec3( 0, 0.5, 0 ), 1.0f } );

                auto sceneNode = scene->createSphereLight( sphereLight );
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "Disk Light" ) ) {
                auto diskLight = g_RenderableEntityManager->createDiskLight( DiskLightData{ glm::vec3( 0, 0.5, 0 ), 1.0f } );

                auto sceneNode = scene->createDiskLight( diskLight );
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "Rectangle/Tube Light" ) ) {
                auto rectangleLight = g_RenderableEntityManager->createRectangleLight( RectangleLightData{ glm::vec3( 0, 0.5, 0 ) } );

                auto sceneNode = scene->createRectangleLight( rectangleLight );
                *PickedNode = sceneNode;
            }

            ImGui::Separator();

            if ( ImGui::MenuItem( "Global Environment Probe" ) ) {
                auto envProbe = g_RenderableEntityManager->createGlobalEnvironmentProbe();

                auto sceneNode = scene->createEnvironmentProbe( envProbe );
                *PickedNode = sceneNode;
            }

            if ( ImGui::MenuItem( "Local Environment Probe" ) ) {
                auto envProbe = g_RenderableEntityManager->createLocalEnvironmentProbe();

                auto sceneNode = scene->createEnvironmentProbe( envProbe );
                *PickedNode = sceneNode;
            }

            //if ( ImGui::MenuItem( "Car Test" ) ) {
            //    auto rootNode = scene->createEmptyNode( "CarTest" );
            //    rootNode->transform.setLocalTranslation( glm::vec3( 0, 10.0f, 0 ) );
            //    rootNode->transform.setLocalScale( glm::vec3( 32.0f ) );
            //    rootNode->rigidBody = new RigidBody( 1600.0f, glm::vec3( 0, 10.0f, 0 ) );
            //    auto nativeRigidBody = rootNode->rigidBody->getNativeObject();
            //    auto file = g_VirtualFileSystem->openFile( FLAN_STRING( "GameData/Geometry/default_car.mesh.hull" ) );

            //    std::vector<float> vertices;
            //    Io_ReadConvexHullColliderFile( file, vertices );
            //    file->close();
            //    delete file;

            //    auto shape = new btConvexHullShape();
            //    for ( int i = 0; i < vertices.size(); i += 3 ) {
            //        shape->addPoint( btVector3( vertices[i], vertices[i + 1], vertices[i + 2] ), false );
            //    }
            //    shape->recalcLocalAabb();

            //    nativeRigidBody->setCollisionShape( shape );

            //    auto vehicleTest = new MotorizedVehicle( rootNode->rigidBody );

            //    g_DynamicsWorld->addMotorizedVehicle( vehicleTest );

            //    // Create Model
            //    auto vehicleModelNode = scene->createModel( g_GraphicsAssetManager->getModel( FLAN_STRING( "GameData/Models/default_car.model" ) ), "VehicleModel", rootNode );
            //    //vehicleModelNode->transform.setLocalTranslation( glm::vec3( 0, -3.0f, 0 ) );

            //    // Create Camera
            //    auto playerCamera = new LookAtCamera();
            //    playerCamera->setProjectionMatrix( 90.0f, 1920.0f, 1080.0f );
            //    auto playerCameraNode = scene->createLookAtCamera( playerCamera, "PlayerCamera", rootNode );

            //    // Create GameLogic
            //    auto playerLogic = new VehicleLogic( vehicleTest );
            //    auto palyerLogicNode = scene->createGameObject( playerLogic, "PlayerLogic", rootNode );
            //    *PickedNode = rootNode;
            //}


            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( "Debug" ) ) {
            ImGui::Checkbox( "Display Bounding Primitives", DrawBoundingPrimitive );
            ImGui::Checkbox( "Print Viewport Infos", PrintViewportInfos );
            ImGui::Checkbox( "Enable CPU Profiler Print", EnableCPUProfilerPrint );
            ImGui::Checkbox( "Enable RenderPipeline Profiler Print", EnablePipelineProfiling );
            ImGui::Checkbox( "Enable CPU Framerate Print", EnableCPUFPSPrint );
            ImGui::Checkbox( "Display Dynamics World Colliders", EnableDebugPhysicsColliders );
            ImGui::Checkbox( "Display Tile Heat", DisplayTileHeat );
            ImGui::Checkbox( "Display Grass LOD Debug Color", GrassLODDebugView );
            ImGui::Checkbox( "Print Memory Debug Infos", PrintMemoryDebugInfos );
            
            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( "Graphics" ) ) {
            if ( ImGui::BeginMenu( "MSAA" ) ) {
                FLAN_IMPORT_VAR_PTR( MSAASamplerCount, int32_t );
                if ( ImGui::MenuItem( "x1", nullptr, *MSAASamplerCount == 1 ) ) {
                    *MSAASamplerCount = 1;
                }
                if ( ImGui::MenuItem( "x2", nullptr, *MSAASamplerCount == 2 ) ) {
                    *MSAASamplerCount = 2;
                }
                if ( ImGui::MenuItem( "x4", nullptr, *MSAASamplerCount == 4 ) ) {
                    *MSAASamplerCount = 4;
                }
                if ( ImGui::MenuItem( "x8", nullptr, *MSAASamplerCount == 8 ) ) {
                    *MSAASamplerCount = 8;
                }
                ImGui::EndMenu();
            }

            if ( ImGui::BeginMenu( "SSAA" ) ) {
                FLAN_IMPORT_VAR_PTR( SSAAMultiplicator, float );
                if ( ImGui::MenuItem( "x1.0", nullptr, *SSAAMultiplicator == 1.0f ) ) {
                    *SSAAMultiplicator = 1.0f;
                }
                if ( ImGui::MenuItem( "x1.5", nullptr, *SSAAMultiplicator == 1.5f ) ) {
                    *SSAAMultiplicator = 1.5f;
                }
                if ( ImGui::MenuItem( "x2.0", nullptr, *SSAAMultiplicator == 2.0f ) ) {
                    *SSAAMultiplicator = 2.0f;
                }
                if ( ImGui::MenuItem( "x4", nullptr, *SSAAMultiplicator == 4.0f ) ) {
                    *SSAAMultiplicator = 4.0f;
                }
                if ( ImGui::MenuItem( "x8", nullptr, *SSAAMultiplicator == 8.0f ) ) {
                    *SSAAMultiplicator = 8.0f;
                }

                ImGui::EndMenu();
            }

            if ( ImGui::Checkbox( "Enable VSync", EnableVSync ) ) {
                g_RenderDevice->setVSyncState( *EnableVSync );
            }

            ImGui::Checkbox( "Enable Temporal AA", EnableTemporalAA );
            ImGui::Checkbox( "Enable FXAA", EnableFXAA );

            ImGui::DragFloat( "Bloom Exposure Compensation", &GraphicsCompositionSettings->BloomExposureCompensation );
            ImGui::SliderFloat( "Bloom Strength", &GraphicsCompositionSettings->BloomStrength, 0.0f, 1.0f );
            ImGui::SliderFloat( "White Point", &GraphicsCompositionSettings->WhitePoint, 0.0f, 1.0f );
            ImGui::SliderFloat( "Black Point", &GraphicsCompositionSettings->BlackPoint, 0.0f, 1.0f );
            ImGui::SliderFloat( "Junction Point", &GraphicsCompositionSettings->JunctionPoint, 0.0f, 1.0f );
            ImGui::SliderFloat( "Toe Strength", &GraphicsCompositionSettings->ToeStrength, 0.0f, 1.0f );
            ImGui::SliderFloat( "Shoulder Strength", &GraphicsCompositionSettings->ShoulderStrength, 0.0f, 1.0f );

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( "HUD" ) ) {
            FLAN_IMPORT_VAR_PTR( ScreenSafeAreaMargin, float );
            ImGui::DragFloat( "Screen Safe Area Margin", ScreenSafeAreaMargin, 0.01f, 0.01f, 1.0f );

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
