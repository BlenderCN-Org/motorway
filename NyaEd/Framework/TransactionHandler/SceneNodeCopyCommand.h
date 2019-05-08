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

#include "TransactionCommand.h"

#include <Framework/Scene.h>
//#include <Framework/SceneNodes/SceneNode.h>
//#include <Physics/DynamicsWorld.h>
//
//class SceneNodeCopyCommand : public TransactionCommand
//{
//public:
//    SceneNodeCopyCommand( SceneNode* nodeToCopy, Scene* currentScene, RenderableEntityManager* currentRenderEntityMan, DynamicsWorld* currentDynamicsWorld )
//        : node( nodeToCopy )
//        , copiedNode( nullptr )
//        , scene( currentScene )
//        , renderableEntityManager( currentRenderEntityMan )
//        , dynamicsWorld( currentDynamicsWorld )
//    {
//        actionInfos = "Copy Node '" + node->name + "'";
//    }
//
//    virtual void execute() override
//    {
//        RigidBody* copiedRigidBody = nullptr;
//
//        if ( node->rigidBody != nullptr ) {
//            copiedRigidBody = new RigidBody( *node->rigidBody );
//            dynamicsWorld->addRigidBody( copiedRigidBody );
//        }
//
//        copiedNode = scene->addNode( node->clone( renderableEntityManager ) );
//        copiedNode->rigidBody = copiedRigidBody;
//
//        FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* );
//        *PickedNode = copiedNode;
//    }
//
//    virtual void undo() override
//    {
//        if ( copiedNode != nullptr ) {
//            scene->removeNode( copiedNode->hashcode );
//
//            if ( copiedNode->rigidBody != nullptr ) {
//                dynamicsWorld->removeRigidBody( copiedNode->rigidBody );
//                delete copiedNode->rigidBody;
//                copiedNode->rigidBody = nullptr;
//            }
//        }
//
//        FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* );
//        *PickedNode = node;
//    }
//
//private:
//    SceneNode*                  node;
//    SceneNode*                  copiedNode;
//    Scene*                      scene;
//    RenderableEntityManager*    renderableEntityManager;
//    DynamicsWorld*              dynamicsWorld;
//};
