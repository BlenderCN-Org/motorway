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
#include "AppShared.h"

#include "GameLogic.h"

#include <Input/InputMapper.h>
#include <Input/InputReader.h>

#include <Framework/Light.h>
#include <Framework/Scene.h>

#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/DrawCommandBuilder.h>
#include <Graphics/RenderableEntityManager.h>

#include <Display/DisplaySurface.h>
#include <Core/Maths/CoordinatesSystemHelpers.h>

#if FLAN_DEVBUILD
//#include <Framework/DebugUI/DebugUI.h>

#include <FileSystem/FileSystemObjectNative.h>
#include <Io/Scene.h>

#include <Framework/TransactionHandler/TransactionHandler.h>
#include <Framework/TransactionHandler/SceneNodeCopyCommand.h>
#include <Framework/TransactionHandler/SceneNodeDeleteCommand.h>
#endif

#include <Framework/SceneNodes/EmptySceneNode.h>
#include "VehicleLogic.h"

// Graphics
FLAN_ENV_VAR( CameraFOV, "Camera FieldOfView (in degrees)", 80.0f, float )
FLAN_ENV_VAR( MSAASamplerCount, "Defines MSAA sampler count [0/2/4/8]", 0, int32_t )
FLAN_ENV_VAR( EnableTemporalAA, "Enables Temporal Antialiasing [false/true]", false, bool )
FLAN_ENV_VAR( EnableFXAA, "Enables FXAA [false/true]", false, bool )

FLAN_DEV_VAR( CopiedNode, "Copied Node", nullptr, SceneNode* )
FLAN_DEV_VAR( PickedNode, "Picked Node", nullptr, SceneNode* )
FLAN_DEV_VAR( IsDevMenuVisible, "IsDevMenuVisible [false/true]", false, bool )

FLAN_DEV_VAR_PERSISTENT( EditorAutoSaveDelayInSeconds, "Auto save delay (in seconds)", 120.0f, float )

GameLogic::GameLogic()
    : currentMode( eGameMode::SANDBOX )
    , currentScene( nullptr )
{

}

GameLogic::~GameLogic()
{

}

void GameLogic::create()
{
    // TODO Remove this once menu and UI are set
#if FLAN_DEVBUILD
    g_InputMapper->pushContext( FLAN_STRING_HASH( "Editor" ) );

    // Free Camera
    g_InputMapper->addCallback( [&]( MappedInput& input, float frameTime ) {
        if ( !IsDevMenuVisible ) {
            auto cameraNode = ( FreeCameraSceneNode* )g_GameLogic->getCurrentScene()->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) );
            auto mainCamera = cameraNode->camera;

            // Camera Controls
            auto axisX = input.Ranges[FLAN_STRING_HASH( "CameraMoveHorizontal" )];
            auto axisY = input.Ranges[FLAN_STRING_HASH( "CameraMoveVertical" )];

            mainCamera->OnMouseUpdate( frameTime, axisX, axisY );

            if ( input.States.find( FLAN_STRING_HASH( "CameraMoveRight" ) ) != input.States.end() ) {
                mainCamera->MoveRight( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "CameraMoveLeft" ) ) != input.States.end() ) {
                mainCamera->MoveLeft( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "CameraMoveForward" ) ) != input.States.end() ) {
                mainCamera->MoveForward( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "CameraMoveBackward" ) ) != input.States.end() ) {
                mainCamera->MoveBackward( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "CameraLowerAltitude" ) ) != input.States.end() ) {
                mainCamera->LowerAltitude( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "CameraTakeAltitude" ) ) != input.States.end() ) {
                mainCamera->TakeAltitude( frameTime );
            }
        }
    }, 0 );

    // Debug UI
    g_InputMapper->addCallback( [&]( MappedInput& input, float frameTime ) {
        // DebugUI
        if ( input.Actions.find( FLAN_STRING_HASH( "OpenDevMenu" ) ) != input.Actions.end() ) {
            IsDevMenuVisible = !IsDevMenuVisible;

            if ( IsDevMenuVisible ) {
                currentMode = eGameMode::DEV_MENU;
                g_InputMapper->pushContext( FLAN_STRING_HASH( "DebugUI" ) );
            } else {
                currentMode = eGameMode::SANDBOX;
                g_InputMapper->popContext();
            }
        }

        if ( input.Actions.find( FLAN_STRING_HASH( "SwitchToGame" ) ) != input.Actions.end() ) {
            if ( currentMode != eGameMode::GAMEPLAY ) {
                currentMode = eGameMode::GAMEPLAY;

                auto cameraNode = ( FreeCameraSceneNode* )g_GameLogic->getCurrentScene()->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) );
                if ( cameraNode )
                    cameraNode->enabled = false;

                auto playerCamera = ( LookAtCameraSceneNode* )g_GameLogic->getCurrentScene()->findNodeByHashcode( FLAN_STRING_HASH( "PlayerCamera" ) );
                if ( playerCamera )
                    playerCamera->enabled = true;

                g_InputMapper->pushContext( FLAN_STRING_HASH( "Game" ) );
            } else {
                g_InputMapper->popContext();
                currentMode = eGameMode::DEV_MENU;

                auto cameraNode = ( FreeCameraSceneNode* )g_GameLogic->getCurrentScene()->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) );
                if ( cameraNode )
                    cameraNode->enabled = true;

                auto playerCamera = ( LookAtCameraSceneNode* )g_GameLogic->getCurrentScene()->findNodeByHashcode( FLAN_STRING_HASH( "PlayerCamera" ) );
                if ( playerCamera )
                    playerCamera->enabled = false;

                g_InputMapper->pushContext( FLAN_STRING_HASH( "Editor" ) );
                g_InputMapper->pushContext( FLAN_STRING_HASH( "DebugUI" ) );
            }
        }

        auto cameraNode = ( FreeCameraSceneNode* )g_GameLogic->getCurrentScene()->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) );

        // Compatibility Hack; to remove later
        if ( cameraNode != nullptr ) {
            auto mainCamera = cameraNode->camera;

            if ( input.States.find( FLAN_STRING_HASH( "MoveCamera" ) ) != input.States.end() ) {
                auto axisX = input.Ranges[FLAN_STRING_HASH( "CameraMoveHorizontal" )];
                auto axisY = input.Ranges[FLAN_STRING_HASH( "CameraMoveVertical" )];

                mainCamera->OnMouseUpdate( frameTime, axisX, axisY );
            }
            ImGuiIO& io = ImGui::GetIO();

            if ( !io.WantCaptureMouse ) {
                if ( input.Actions.find( FLAN_STRING_HASH( "PickNode" ) ) != input.Actions.end() ) {
                    auto cameraData = mainCamera->GetData();
                    auto viewMat = glm::transpose( cameraData.viewMatrix );
                    auto projMat = cameraData.depthProjectionMatrix;

                    auto inverseViewProj = glm::inverse( projMat * viewMat );

                    glm::vec4 ray =
                    {
                        ( io.MousePos.x / io.DisplaySize.x ) * 2.f - 1.f,
                        ( 1.f - ( io.MousePos.y / io.DisplaySize.y ) ) * 2.f - 1.f,
                        0.0f,
                        1.0f
                    };

                    glm::vec4 rayOrigin =
                    {
                        ray.x * inverseViewProj[0][0] + ray.y * inverseViewProj[1][0] + ray.z * inverseViewProj[2][0] + ray.w * inverseViewProj[3][0],
                        ray.x * inverseViewProj[0][1] + ray.y * inverseViewProj[1][1] + ray.z * inverseViewProj[2][1] + ray.w * inverseViewProj[3][1],
                        ray.x * inverseViewProj[0][2] + ray.y * inverseViewProj[1][2] + ray.z * inverseViewProj[2][2] + ray.w * inverseViewProj[3][2],
                        ray.x * inverseViewProj[0][3] + ray.y * inverseViewProj[1][3] + ray.z * inverseViewProj[2][3] + ray.w * inverseViewProj[3][3],
                    };
                    rayOrigin *= ( 1.0f / rayOrigin.w );

                    ray.z = 1.0f;
                    glm::vec4 rayEnd =
                    {
                        ray.x * inverseViewProj[0][0] + ray.y * inverseViewProj[1][0] + ray.z * inverseViewProj[2][0] + ray.w * inverseViewProj[3][0],
                        ray.x * inverseViewProj[0][1] + ray.y * inverseViewProj[1][1] + ray.z * inverseViewProj[2][1] + ray.w * inverseViewProj[3][1],
                        ray.x * inverseViewProj[0][2] + ray.y * inverseViewProj[1][2] + ray.z * inverseViewProj[2][2] + ray.w * inverseViewProj[3][2],
                        ray.x * inverseViewProj[0][3] + ray.y * inverseViewProj[1][3] + ray.z * inverseViewProj[2][3] + ray.w * inverseViewProj[3][3],
                    };
                    rayEnd *= ( 1.0f / rayEnd.w );

                    auto rayDir = glm::normalize( rayEnd - rayOrigin );

                    glm::vec3 rayDirection = glm::vec3( rayDir );
                    glm::vec3 rayOrig = glm::vec3( rayOrigin );

                    Ray rayObj( rayOrig, rayDirection );
                    PickedNode = getCurrentScene()->intersect( rayObj );
                }
            }
        }

        // Default: Ctrl
        if ( input.States.find( FLAN_STRING_HASH( "Modifier1" ) ) != input.States.end() ) {
            if ( input.Actions.find( FLAN_STRING_HASH( "CopyNode" ) ) != input.Actions.end() ) {
                if ( PickedNode != nullptr ) {
                    CopiedNode = PickedNode;
                }
            } 

            if ( input.Actions.find( FLAN_STRING_HASH( "OpenScene" ) ) != input.Actions.end() ) {
                fnString_t sceneName;
                if ( flan::core::DisplayFileOpenPrompt( sceneName, FLAN_STRING( "Asset Scene file (*.scene)\0*.scene" ), FLAN_STRING( "./" ), FLAN_STRING( "Save as a Scene asset" ) ) ) {
                    auto file = new FileSystemObjectNative( sceneName );
                    file->open( std::ios::binary | std::ios::in );

                    PickedNode = nullptr;
                    g_RenderableEntityManager->clear();

                    Scene* loadedScene = new Scene();

                    // Trigger scene change (flush CPU/GPU buffers; discard current game state; etc.)
                    g_GameLogic->changeScene( loadedScene );

                    // Then load the scene
                    Io_ReadSceneFile( file, g_GraphicsAssetManager.get(), g_RenderableEntityManager.get(), *loadedScene );
                    file->close();
                    delete file;

                    for ( auto& node : g_GameLogic->getCurrentScene()->getSceneNodes() ) {
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

            if ( input.Actions.find( FLAN_STRING_HASH( "SaveScene" ) ) != input.Actions.end() ) {
                fnString_t sceneName;
                if ( flan::core::DisplayFileSavePrompt( sceneName, FLAN_STRING( "Asset Scene file (*.scene)\0*.scene" ), FLAN_STRING( "./" ), FLAN_STRING( "Save as a Scene asset" ) ) ) {
                    sceneName = fnString_t( sceneName.c_str() );
                    auto file = new FileSystemObjectNative( fnString_t( sceneName + FLAN_STRING( ".scene" ) ) );
                    file->open( std::ios::binary | std::ios::out );
                    Io_WriteSceneFile( g_GameLogic->getCurrentScene(), file );
                    file->close();
                    delete file;
                }
            }
            
            if ( input.Actions.find( FLAN_STRING_HASH( "PasteNode" ) ) != input.Actions.end() ) {
                if ( CopiedNode != nullptr ) {
                    g_TransactionHandler->commit( new SceneNodeCopyCommand( PickedNode, getCurrentScene(), g_RenderableEntityManager.get(), g_DynamicsWorld.get() ) );
                }
            }

            if ( input.Actions.find( FLAN_STRING_HASH( "Undo" ) ) != input.Actions.end() ) {
                g_TransactionHandler->undo();
            }

            if ( input.Actions.find( FLAN_STRING_HASH( "Redo" ) ) != input.Actions.end() ) {
                g_TransactionHandler->redo();
            }
        }

        if ( input.Actions.find( FLAN_STRING_HASH( "DeleteNode" ) ) != input.Actions.end() ) {
            if ( PickedNode != nullptr ) {
                g_TransactionHandler->commit( new SceneNodeDeleteCommand( PickedNode, getCurrentScene(), g_RenderableEntityManager.get(), g_DynamicsWorld.get() ) );
                PickedNode = nullptr;
            }
        }
    }, -1 );

    autoSaveTimer.start();
#endif

    // Game
    g_InputMapper->addCallback( [&]( MappedInput& input, float frameTime ) {
        auto playerNode = ( EmptySceneNode* )g_GameLogic->getCurrentScene()->findNodeByHashcode( FLAN_STRING_HASH( "CarTest" ) );

        if ( playerNode != nullptr ) {
            auto playerNodeRigidBody = playerNode->rigidBody;

            auto playerCamera = (LookAtCameraSceneNode*)playerNode->findChildByHashcode( FLAN_STRING_HASH( "PlayerCamera" ) );

            // Camera Controls
            auto axisX = input.Ranges[FLAN_STRING_HASH( "HeadMoveHorizontal" )];
            auto axisY = input.Ranges[FLAN_STRING_HASH( "HeadMoveVertical" )];

            auto camera = static_cast< LookAtCamera* >( playerCamera->camera );
            camera->isMovingCamera( input.States.find( FLAN_STRING_HASH( "MoveCamera" ) ) != input.States.end() );
          
            playerCamera->camera->onMouseUpdate( frameTime, axisX, axisY );

            auto playerLogic = ( GameObjectSceneNode* )playerNode->findChildByHashcode( FLAN_STRING_HASH( "PlayerLogic" ) );
            auto playerLogicScript = ( VehicleLogic* )playerLogic->gameObject;

            if ( input.States.find( FLAN_STRING_HASH( "Accelerate" ) ) != input.States.end() ) {
                playerLogicScript->Accelerate( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "Brake" ) ) != input.States.end() ) {
                playerLogicScript->Brake( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "SteerLeft" ) ) != input.States.end() ) {
                playerLogicScript->Steer( frameTime, -1.0f );
            }

            if ( input.States.find( FLAN_STRING_HASH( "SteerRight" ) ) != input.States.end() ) {
                playerLogicScript->Steer( frameTime, 1.0f );
            }
        }
    }, 1 );

    loadDefaultScene();
}

void GameLogic::update( const float frameTime )
{
    Camera* mainCamera = nullptr;

    // Find active camera (either gameplay camera or the default/ed one)
    auto playerCameraNode = static_cast< LookAtCameraSceneNode* >( currentScene->findNodeByHashcode( FLAN_STRING_HASH( "PlayerCamera" ) ) );
    if ( playerCameraNode != nullptr && currentMode == eGameMode::GAMEPLAY ) {
        mainCamera = playerCameraNode->camera;
    } else {
        // Fallback to freecam if non cam is set (even if we are in gameplay mode)
        auto cameraNode = static_cast< FreeCameraSceneNode* >( currentScene->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) ) );
        if ( cameraNode != nullptr ) {
            mainCamera = cameraNode->camera;
        }
    }

    // Explicit pipeline rebuild each frame (to allow render pass hotswapping)
    if ( mainCamera != nullptr ) {
        rebuildPlayerRenderPipeline( mainCamera );
        mainCamera->Update( frameTime );
    }

    // Update current gamemode
    switch ( currentMode ) {
    case eGameMode::GAMEPLAY: {
        if ( playerCameraNode == nullptr ) {
            g_DrawCommandBuilder->addHUDText( "MISSING GAMEPLAY CAMERA ('PlayerCamera' node not found)", 0.5f, 0.0f, 0.5f, 0.0f, glm::vec4( 1, 0, 0, 1 ) );
            auto cameraNode = static_cast< FreeCameraSceneNode* >( currentScene->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) ) );
            if ( cameraNode != nullptr ) {
                cameraNode->enabled = true;
            }
            changeGameMode( eGameMode::SANDBOX );
        } else {
            auto camera = playerCameraNode->camera;
            auto model = static_cast< ModelSceneNode* >( currentScene->findNodeByHashcode( FLAN_STRING_HASH( "VehicleModel" ) ) );

            camera->setNextWorldPosition( frameTime, model->transform.getWorldTranslation() );
            camera->setNextWorldRotation( frameTime, model->transform.getWorldRotation() );

            g_MainDisplaySurface->setMousePosition( 0.5f, 0.5f );
        }
    } break;

    case eGameMode::SANDBOX:
        g_MainDisplaySurface->setMousePosition( 0.5f, 0.5f );

        if ( autoSaveTimer.getElapsedAsSeconds() > EditorAutoSaveDelayInSeconds ) {
            FLAN_CLOG << "Autosaving started! Hope it doesn't crash..." << std::endl;

            auto file = new FileSystemObjectNative( FLAN_STRING( "./data/autosaved.scene" ) );
            file->open( std::ios::binary | std::ios::out );
            Io_WriteSceneFile( g_GameLogic->getCurrentScene(), file );
            file->close();
            delete file;

            autoSaveTimer.reset();
        }
        break;

    case eGameMode::DEV_MENU:
        break;
    }

    // Update scene nodes
    currentScene->update( frameTime );
}

void GameLogic::collectRenderKeys( DrawCommandBuilder* drawCommandBuilder )
{
    currentScene->collectRenderKeys( drawCommandBuilder );
}

void GameLogic::changeScene( Scene* newScene )
{
    FLAN_CLOG << "Next scene: '" << newScene->getSceneName().c_str() << "'" << std::endl;

    if ( currentScene != nullptr ) {
        for ( auto& node : currentScene->getSceneNodes() ) {
            node->remove( g_RenderableEntityManager.get() );

            if ( node->rigidBody != nullptr ) {
                g_DynamicsWorld->removeRigidBody( node->rigidBody );
                delete node->rigidBody;
            }
        }

        g_RenderableEntityManager->clear();
    }

    currentScene.reset( newScene );
}

void GameLogic::changeGameMode( const eGameMode nextGameMode )
{
    FLAN_CLOG << "Next game mode: '" << nextGameMode << "'" << std::endl;

    currentMode = nextGameMode;
}

void GameLogic::loadDefaultScene()
{
    currentScene.reset( new Scene() );

    // Setup at least one base camera
    uint32_t surfaceWidth = 0, surfaceHeight = 0;
    g_MainDisplaySurface->getSurfaceDimension( surfaceWidth, surfaceHeight );

    auto testCam = new FreeCamera();
    testCam->SetProjectionMatrix( CameraFOV, static_cast<float>( surfaceWidth ), static_cast<float>( surfaceHeight ) );
    auto testCamNode = ( FreeCameraSceneNode* )currentScene->createFreeCamera( testCam, "DefaultCamera" );
    testCamNode->enabled = true;
}

void GameLogic::rebuildPlayerRenderPipeline( Camera* mainCamera )
{
    mainCamera->clearRenderPasses();
    mainCamera->addRenderPass( FLAN_STRING_HASH( "CascadedShadowMapCapture" ) );
    mainCamera->addRenderPass( FLAN_STRING_HASH( "AtmosphereRenderPass" ) );

    if ( MSAASamplerCount <= 1 ) {
        mainCamera->addRenderPass( FLAN_STRING_HASH( "WorldDepthPass" ) );
        mainCamera->addRenderPass( FLAN_STRING_HASH( "LightCullingPass" ) );
        mainCamera->addRenderPass( FLAN_STRING_HASH( "WorldLightPass" ) );
        mainCamera->addRenderPass( FLAN_STRING_HASH( "DebugWorldPass" ) );
        mainCamera->addRenderPass( FLAN_STRING_HASH( "LineRenderPass" ) );
    } else {
        mainCamera->addRenderPass( FLAN_STRING_HASH( "CopyTextureToMSAAPass" ) );
        mainCamera->addRenderPass( FLAN_STRING_HASH( "WorldDepthMSAAPass" ) );
        mainCamera->addRenderPass( FLAN_STRING_HASH( "LightCullingMSAAPass" ) );
        mainCamera->addRenderPass( FLAN_STRING_HASH( "WorldLightMSAAPass" ) );
        mainCamera->addRenderPass( FLAN_STRING_HASH( "DebugWorldMSAAPass" ) );
        mainCamera->addRenderPass( FLAN_STRING_HASH( "LineRenderPass" ) );

        mainCamera->addRenderPass( FLAN_STRING_HASH( std::string( "MSAADepthResolvePass" + std::to_string( MSAASamplerCount ) ).c_str() ) );

        if ( EnableTemporalAA ) {
            mainCamera->addRenderPass( FLAN_STRING_HASH( std::string( "AntiAliasingPassMSAA" + std::to_string( MSAASamplerCount ) + "TAA" ).c_str() ) );
        } else {
            mainCamera->addRenderPass( FLAN_STRING_HASH( std::string( "AntiAliasingPassMSAA" + std::to_string( MSAASamplerCount ) ).c_str() ) );
        }
    }

    mainCamera->addRenderPass( FLAN_STRING_HASH( "AutoExposurePass" ) );
    mainCamera->addRenderPass( FLAN_STRING_HASH( "BloomPass" ) );
    mainCamera->addRenderPass( FLAN_STRING_HASH( "CompositionPass" ) );

    if ( EnableFXAA ) {
        mainCamera->addRenderPass( FLAN_STRING_HASH( "FXAAPass" ) );
    }

    mainCamera->addRenderPass( FLAN_STRING_HASH( "HUDRenderPass" ) );
    mainCamera->addRenderPass( FLAN_STRING_HASH( "TextRenderPass" ) );
    mainCamera->addRenderPass( FLAN_STRING_HASH( "PresentPass" ) );
}
