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

#include "Player.h"

#include <Framework/Scene.h>

#include <Physics/DynamicsWorld.h>
#include <Graphics/GraphicsAssetManager.h>

SceneNode* Player::addToScene( Scene* scene )
{
    // Setup Root
    auto playerRootNode = scene->createEmptyNode( "Player" );

    playerRootNode->rigidBody = new RigidBody( 700.0f );
    auto nativeRigidBody = playerRootNode->rigidBody->getNativeObject();
    nativeRigidBody->setCollisionShape( new btCapsuleShape( 1.00f, 2.00f ) );
    playerRootNode->rigidBody->recomputeInertia();

    g_DynamicsWorld->addRigidBody( playerRootNode->rigidBody );

    // Create Model
    auto playerModelNode = scene->createModel( g_GraphicsAssetManager->getModel( FLAN_STRING( "GameData/Models/player_placeholder.model" ) ), "PlayerModel", playerRootNode );
    playerModelNode->transform.setLocalTranslation( glm::vec3( 0, -3.0f, 0 ) );

    // Create Camera
    auto playerCamera = new FirstPersonCamera();
    playerCamera->setProjectionMatrix( 90.0f, 1920.0f, 1080.0f );
    auto playerCameraNode = scene->createFirstPersonCamera( playerCamera, "PlayerCamera", playerRootNode );

    return playerRootNode;
}
