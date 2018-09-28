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

#include "EditorInterface.h"

#include "AppShared.h"
#include <Framework/SceneNodes/SceneNode.h>
#include <Graphics/WorldRenderer.h>

#ifdef FLAN_DEVBUILD
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>

#include <Physics/DynamicsWorld.h>

#include <Framework/Material.h>
#include <FileSystem/FileSystemObjectNative.h>

#include <Display/DisplaySurfaceWin32.h>
#include <Io/Scene.h>
#include <Io/Collider.h>

#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_win32.h>

#include <FileSystem/VirtualFileSystem.h>

#if FLAN_D3D11
#include <Rendering/Direct3D11/CommandList.h>
#include <Rendering/Direct3D11/CommandListPool.h>
#include <Rendering/Direct3D11/RenderContext.h>
#include <imgui/examples/imgui_impl_dx11.h>
#elif FLAN_VULKAN
#include <Rendering/Vulkan/CommandList.h>
#include <Rendering/Vulkan/CommandListPool.h>
#include <Rendering/Vulkan/RenderContext.h>
#include <imgui/examples/imgui_impl_vulkan.h>
#elif FLAN_GL460
#include <Rendering/OpenGL460/CommandList.h>
#include <Rendering/OpenGL460/CommandListPool.h>
#include <Rendering/OpenGL460/RenderContext.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#endif

#include <Rendering/CommandListPool.h>
#include <Rendering/CommandList.h>
#include <Rendering/RenderDevice.h>

#include <Framework/Scene.h>

#include <Framework/TransactionHandler/TransactionHandler.h>
#include <Framework/TransactionHandler/SceneNodeCopyCommand.h>
#include <Framework/TransactionHandler/SceneNodeDeleteCommand.h>

static int panelId = 0;

FLAN_DEV_VAR( dev_GuizmoViewMatrix, "Transform Guizmo ViewMatrix", nullptr, float* )
FLAN_DEV_VAR( dev_GuizmoProjMatrix, "Transform Guizmo ProjectionMatrix", nullptr, float* )
FLAN_DEV_VAR( dev_EditorPickedMaterial, "Material Picked in the Material Editor", nullptr, Material* )
FLAN_DEV_VAR( dev_IsInputText, "Is Using a Text Input", false, bool )

static void PrintNode( SceneNode* node )
{
    FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* )
    ImGuiTreeNodeFlags flags = 0;

    const bool isLeaf = node->children.empty();
    const bool isSelected = ( ( *PickedNode ) != nullptr && ( *PickedNode )->hashcode == node->hashcode );

    if ( isSelected ) 
        flags |= ImGuiTreeNodeFlags_Selected;

    if ( isLeaf ) 
        flags |= ImGuiTreeNodeFlags_Leaf;

    bool isOpened = ImGui::TreeNodeEx( node->name.c_str(), flags );

    if ( ImGui::IsItemClicked() ) {
        *PickedNode = node;
    }

    if ( isOpened ) {
        if ( !isLeaf ) {
            ImGui::Indent();
            for ( auto child : node->children ) {
                PrintNode( child );
            }
            ImGui::Unindent();
        }

        ImGui::TreePop();
    }
}

static void PrintTab( const char* tabName, const int tabIndex )
{
    if ( panelId == tabIndex ) {
        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.96f, 0.62f, 0.1f, 1.0f ) );
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.1f, 0.1f, 0.1f, 1.0f ) );
        if ( ImGui::Button( tabName ) ) {
            panelId = tabIndex;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    } else {
        if ( ImGui::Button( tabName ) ) {
            panelId = tabIndex;
        }
    }
}

static void DisplayMenuBar()
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
    FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* )
    FLAN_IMPORT_VAR_PTR( CopiedNode, SceneNode* )

    if ( ImGui::BeginMainMenuBar() ) {
        if ( ImGui::BeginMenu( "File" ) ) {
            if ( ImGui::MenuItem( "New Scene" ) ) {
                *PickedNode = nullptr;
                g_RenderableEntityManager->clear();
                g_CurrentScene.reset( new Scene() );

                auto camera = new FreeCamera();
                camera->SetProjectionMatrix( 80.0f, 1920.0f, 1080.0f );

                auto sceneNode = (FreeCameraSceneNode*)g_CurrentScene->createFreeCamera( camera, "DefaultCamera" );
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
                    g_CurrentScene.reset( loadedScene );

                    // Then load the scene
                    Io_ReadSceneFile( file, g_GraphicsAssetManager.get(), g_RenderableEntityManager.get(), *loadedScene );
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

                    //    g_RenderManager->LoadProbesFromDisk( g_GraphicsAssetManager.get(), paString_t( sceneNameWithExt.c_str() ) );

                    //    // Rebuild BVH
                }
            }

            if ( ImGui::MenuItem( "Save Scene..." ) ) {
                fnString_t sceneName;
                if ( flan::core::DisplayFileSavePrompt( sceneName, FLAN_STRING( "Asset Scene file (*.scene)\0*.scene" ), FLAN_STRING( "./" ), FLAN_STRING( "Save as a Scene asset" ) ) ) {
                    sceneName = fnString_t( sceneName.c_str() );
                    auto file = new FileSystemObjectNative( fnString_t( sceneName + FLAN_STRING( ".scene" ) ) );
                    file->open( std::ios::binary | std::ios::out );
                    Io_WriteSceneFile( g_CurrentScene.get(), file );
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
                    g_TransactionHandler->commit( new SceneNodeCopyCommand( *PickedNode, g_CurrentScene.get(), g_RenderableEntityManager.get(), g_DynamicsWorld.get() ) );
                }
            }

            if ( ImGui::MenuItem( "Delete" ) ) {
                if ( PickedNode != nullptr ) {
                    g_TransactionHandler->commit( new SceneNodeDeleteCommand( *PickedNode, g_CurrentScene.get(), g_RenderableEntityManager.get(), g_DynamicsWorld.get() ) );
                    PickedNode = nullptr;
                }
            }
            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( "Add To Scene" ) ) {
            auto scene = g_CurrentScene.get();

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

            ImGui::Separator();

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
                FLAN_IMPORT_VAR_PTR( SSAAMultiplicator, int32_t );
                if ( ImGui::MenuItem( "x1.0", nullptr, *SSAAMultiplicator == 1 ) ) {
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

void DrawEditorInterface( const float frameTime, CommandList* cmdList )
{
    const auto& nativeContext = g_RenderDevice->getNativeRenderContext();
    const auto nativeCmdList = cmdList->getNativeCommandList();
    cmdList->beginCommandList( g_RenderDevice.get() );
    cmdList->bindBackbufferCmd();
    
    ImGui_ImplWin32_NewFrame();

#if FLAN_D3D11
    ImGui_ImplDX11_Init( nativeContext->nativeDevice, nativeCmdList->deferredContext );
    ImGui_ImplDX11_NewFrame();
#elif FLAN_GL460
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplOpenGL3_NewFrame();
#elif FLAN_VULKAN
    //ImGui_ImplVulkan_Init( nativeContext->nativeDevice, nativeCmdList->deferredContext );
    ImGui_ImplVulkan_NewFrame();
#endif

    ImGui::NewFrame();

    FLAN_IMPORT_VAR_PTR( IsDevMenuVisible, bool )

    if ( *IsDevMenuVisible ) {
        ImGuizmo::BeginFrame();

        DisplayMenuBar();

        // Update Guizmo Matrices
        auto cameraNode = (FreeCameraSceneNode*)g_CurrentScene.get()->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) );
        if ( cameraNode != nullptr ) {
            auto cameraData = cameraNode->camera->GetData();
            auto transposedView = glm::transpose( cameraData.viewMatrix );
            dev_GuizmoViewMatrix = &transposedView[0][0];
            dev_GuizmoProjMatrix = &cameraData.depthProjectionMatrix[0][0];
        }

        ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0, 0, 0, 0 ) );
        ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0, 0, 0, 0 ) );
        ImGui::PushStyleColor( ImGuiCol_BorderShadow, ImVec4( 0, 0, 0, 0 ) );
        if ( ImGui::Begin( "TabManager", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) ) {
            ImGui::SetWindowPos( ImVec2( 16, 48 ) );
            ImGui::SetWindowSize( ImVec2( 800, 40 ) );
            PrintTab( "NodeEd", 0 );
            ImGui::SameLine( 0, 2 );
            PrintTab( "MaterialEd", 1 );

            ImGui::End();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::PushStyleVar( ImGuiStyleVar_::ImGuiStyleVar_WindowRounding, 0 );
        FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* )
        if ( ImGui::Begin( "Editor Panel", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) ) {
            ImGui::SetWindowPos( ImVec2( 16, 75 ) );
            ImGui::SetWindowSize( ImVec2( 800, 390 ) );
            if ( panelId == 0 ) {
                if ( *PickedNode != nullptr ) {
                    auto node = *PickedNode;

                    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.96f, 0.1f, 0.05f, 1.0f ) );
                    if ( ImGui::Button( "Delete!" ) ) {
                        ImGui::PopStyleColor();

                        g_TransactionHandler->commit( new SceneNodeDeleteCommand( node, g_CurrentScene.get(), g_RenderableEntityManager.get(), g_DynamicsWorld.get() ) );
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

                        const auto& sceneNodes = g_CurrentScene.get()->getSceneNodes();

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
                                UNSUPPORTED,
                                UNSUPPORTED,
                                UNSUPPORTED,
                                UNSUPPORTED,
                                STATIC_PLANE,
                            };

                            static constexpr char* COLLISION_SHAPES_LABELS[eRigidBodyShape::COUNT] = {
                                "(empty)", "Box", "Sphere", "Capsule", "Cylinder", "Cone", "Static Plane", "Convex Hull"
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
                                case eRigidBodyShape::CONVEX_HULL:  {
                                    fnString_t convexHullFile;
                                    if ( flan::core::DisplayFileOpenPrompt( convexHullFile, FLAN_STRING( "Mesh Convex Hull file (*.mesh.hull)\0*.mesh.hull" ), FLAN_STRING( "./" ), FLAN_STRING( "Select a Mesh Convex Hull" ) ) ) {
                                        auto file = new FileSystemObjectNative( convexHullFile );
                                        file->open( std::ios::binary | std::ios::in );

                                        std::vector<float> vertices;
                                        Io_ReadConvexHullColliderFile( file, vertices );
                                        file->close();
                                        delete file;

                                        auto shape = new btConvexHullShape();
                                        for ( int i = 0; i <  vertices.size(); i += 3 ) {
                                            shape->addPoint( btVector3( vertices[i], vertices[i + 1], vertices[i + 2] ), false );
                                        }
                                        shape->recalcLocalAabb();

                                        nativeObject->setCollisionShape( shape );

                                        RebuildRigidBody( node->rigidBody );
                                    } } break;

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
                        node->drawInEditor( g_GraphicsAssetManager.get(), g_TransactionHandler.get(), frameTime );
                    }

                } else {
                    dev_IsInputText = false;
                }
            } else if ( panelId == 1 ) {
                if ( dev_EditorPickedMaterial != nullptr ) {
                    dev_EditorPickedMaterial->drawInEditor( g_RenderDevice.get(), g_ShaderStageManager.get(), g_GraphicsAssetManager.get(), g_WorldRenderer.get() );
                } else {
                    if ( ImGui::Button( "New" ) ) {
                        dev_EditorPickedMaterial = new Material();
                    }
                }
            }
        }
        
        if ( ImGui::Begin( "Scene Hiearchy", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) ) {
            ImGui::SetWindowPos( ImVec2( 16, 464 ) );
            ImGui::SetWindowSize( ImVec2( 800, 220 ) );

            ImGui::LabelText( "##Scene Hiearchy", "Scene Hiearchy" );
            ImGui::SameLine( 0, 0 );
            char sceneNodeSearch[256] = { '\0' };
            ImGui::InputText( "##SceneNodeLookup", sceneNodeSearch, 256, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue );
            ImGui::Separator();

            auto scene = g_CurrentScene.get();
            const auto& sceneNodes = scene->getSceneNodes();
            for ( auto& node : sceneNodes ) {
                if ( node->parent == nullptr ) {
                    PrintNode( node );
                }
            }

            ImGui::End();
        }
        ImGui::PopStyleVar();

        ImGui::End();
    }

    ImGui::Render();
#if FLAN_D3D11
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
#elif FLAN_GL460
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
#elif FLAN_VULKAN
    auto nativeCmd = cmdList->getNativeCommandList();
    //ImGui_ImplVulkan_RenderDrawData( ImGui::GetDrawData(), cmdList );
#endif

    cmdList->endCommandList( g_RenderDevice.get() );
    cmdList->playbackCommandList( g_RenderDevice.get() );
}
#endif