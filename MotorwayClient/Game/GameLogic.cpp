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
