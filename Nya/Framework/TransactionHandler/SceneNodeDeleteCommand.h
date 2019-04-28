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
//
//
//class SceneNodeDeleteCommand : public TransactionCommand
//{
//public:
//    SceneNodeDeleteCommand( SceneNode* nodeToDelete, Scene* currentScene, RenderableEntityManager* currentRenderEntityMan, DynamicsWorld* currentDynamicsWorld )
//        : node( nodeToDelete )
//        , copiedNode( nullptr )
//        , scene( currentScene )
//        , renderableEntityManager( currentRenderEntityMan )
//        , dynamicsWorld( currentDynamicsWorld )
//    {
//        actionInfos = "Delete Node '" + node->name + "'";
//    }
//
//    virtual void execute() override
//    {
//        // Make a copy of the node (in case a restore is required)
//        if ( copiedNode != nullptr ) {
//            delete copiedNode;
//            copiedNode = nullptr;
//        }
//        copiedNode = node->clone( renderableEntityManager );
//
//        if ( !node->children.empty() ) {
//            for ( auto& child : node->children ) {
//                child->parent = node->parent;
//            }
//        }
//
//        if ( scene->removeNode( node->hashcode ) ) {
//            // Unregister from any system the node might be registered to
//            node->remove( renderableEntityManager );
//
//            if ( node->rigidBody != nullptr ) {
//                dynamicsWorld->removeRigidBody( node->rigidBody );
//                delete node->rigidBody;
//                node->rigidBody = nullptr;
//            }
//
//            delete node;
//        }
//    }
//
//    virtual void undo() override
//    {
//        scene->addNode( copiedNode );
//
//        if ( copiedNode->rigidBody != nullptr ) {
//            dynamicsWorld->addRigidBody( copiedNode->rigidBody );
//        }
//    }
//
//private:
//    SceneNode*                  node;
//    SceneNode*                  copiedNode;
//    Scene*                      scene;
//    RenderableEntityManager*    renderableEntityManager;
//    DynamicsWorld*              dynamicsWorld;
//};
