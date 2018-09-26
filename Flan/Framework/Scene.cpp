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
#include "Scene.h"

#include "SceneNodes/EmptySceneNode.h"

Scene::Scene( const std::string& sceneName )
    : name( sceneName )
    , canCollectRenderKeys{ 0 }
    , canUpdate{ 0 }
{
    sceneNodes.reserve( 512 );
}

Scene::~Scene()
{
    for ( int i = 0; i < sceneNodes.size(); i++ ) {
        delete sceneNodes[i];
    }

    sceneNodes.clear();
    nodeHelper.clear();
    name.clear();
}

SceneNode* Scene::createEmptyNode( const std::string& nodeName, SceneNode* parent )
{
    sceneNodes.push_back( new EmptySceneNode( nodeName ) );
    auto& pushedNode = sceneNodes.back();

    buildNode( pushedNode, parent );

    return pushedNode;
}

SceneNode* Scene::addNode( SceneNode* node )
{
    sceneNodes.push_back( node );
    buildNode( sceneNodes.back() );
    return sceneNodes.back();
}

void Scene::clearNodes()
{
    sceneNodes.clear();
}

bool Scene::removeNode( const fnStringHash_t nodeHashcode )
{
    auto mapIterator = nodeHelper.find( nodeHashcode );

    if ( mapIterator != nodeHelper.end() ) {
        auto vectorIndex = mapIterator->second;

        sceneNodes.erase( sceneNodes.begin() + vectorIndex, sceneNodes.begin() + vectorIndex + 1 );
        nodeHelper.erase( nodeHashcode );

        // Update hashmap helper
        for ( auto& nodeHelp : nodeHelper ) {
            if ( nodeHelp.second > vectorIndex ) {
                nodeHelp.second--;
            }
        }

        return true;
    }

    return false;
}

SceneNode* Scene::findNodeByHashcode( const fnStringHash_t nodeHashcode ) const
{
    auto mapIterator = nodeHelper.find( nodeHashcode );

    if ( mapIterator != nodeHelper.end() ) {
        auto vectorIndex = mapIterator->second;
        return sceneNodes.at( vectorIndex );
    }

    return nullptr;
}

SceneNode* Scene::intersect( const Ray& ray ) const
{
    float bestIntersectionDist = std::numeric_limits<float>::max();
    SceneNode* pickedNode = nullptr;

    for ( auto node : sceneNodes ) {
        if ( node->canBeIntersected ) {
            float intersectionDist = std::numeric_limits<float>::max();
            auto isIntersected = node->intersect( ray, intersectionDist );

            if ( isIntersected && intersectionDist < bestIntersectionDist ) {
                bestIntersectionDist = intersectionDist;
                pickedNode = node;
            }
        }
    }

    return pickedNode;
}

void Scene::update( const float frameTime )
{
    for ( int i = 0; i < sceneNodes.size(); i++ ) {
        auto* node = sceneNodes[i];

        node->isDirty = node->transform.rebuildModelMatrix();

        const bool isDynamic = ( node->rigidBody != nullptr );
        if ( isDynamic ) {
            // Sync node transform with dynamics world transform if no manual edit has been applied
            if ( !node->isDirty && !node->transform.IsManipulating() ) {
                auto worldPosition = node->rigidBody->getWorldPosition();
                auto worldRotation = node->rigidBody->getWorldRotation();

                node->transform.setLocalTranslation( worldPosition );
                node->transform.setLocalRotation( worldRotation );

                node->isDirty = node->transform.rebuildModelMatrix();
            } else  {
                node->rigidBody->setWorldTransform( node->transform.getWorldTranslation(), node->transform.getWorldRotation() );
            }
        }

        // TODO Shittons of indirection
        // Simplify this
        if ( node->parent != nullptr && node->parent->isDirty ) {
            const auto modelMat = node->parent->transform.getWorldModelMatrix();
            node->transform.propagateParentModelMatrix( *modelMat );
            node->isDirty = true;
        }

        int arrayIdx = i / 64;
        int bitIdx = i % 64;
        if ( ( canUpdate[arrayIdx] & 0xFF ) >> bitIdx ) {
            node->update( frameTime );
        }
    }
}

void Scene::collectRenderKeys( DrawCommandBuilder* drawCmdBuilder )
{
    for ( auto node : sceneNodes ) {
        if ( node->canCollectRenderKeys ) {
            node->collectRenderKeys( drawCmdBuilder );
        }
    }

#if FLAN_DEVBUILD
    for ( auto node : sceneNodes ) {
        if ( node->canCollectDebugRenderKeys ) {
            node->collectDebugRenderKeys( drawCmdBuilder );
        }
    }
#endif
}

void Scene::buildNode( SceneNode* node, SceneNode* parent )
{
    auto nodeIndex = sceneNodes.size() - 1;

    std::string nodeName = node->name;

    auto nodeHelperIt = nodeHelper.find( node->hashcode );

    // Avoid duplicated node name by appending an index to the end of its name
    uint32_t nodeDuplicateIndex = 0;
    std::string newNodeName = nodeName;
    fnStringHash_t newNodeNameHashcode = flan::core::CRC32( newNodeName );
    while ( nodeHelperIt != nodeHelper.end() ) {
        newNodeName = std::string( nodeName.c_str() ) + "(" + std::to_string( ++nodeDuplicateIndex ) + ")";
        newNodeNameHashcode = flan::core::CRC32( newNodeName );

        nodeHelperIt = nodeHelper.find( newNodeNameHashcode );
    }

    node->name = newNodeName;
    node->hashcode = newNodeNameHashcode;

    nodeHelper.insert( std::make_pair( newNodeNameHashcode, nodeIndex ) );

    node->parent = parent;
    
    if ( parent != nullptr ) {
        parent->children.push_back( node );
    }

    node->isDirty = true;
    //node->NodeRigidBody = rigidBody;

    int arrayIdx = static_cast<int>( nodeIndex ) / 64;
    int bitIdx = nodeIndex % 64;
    canCollectRenderKeys[arrayIdx] = ( node->canCollectRenderKeys << bitIdx ) | canCollectRenderKeys[arrayIdx];
    canUpdate[nodeIndex] = node->canUpdate;
}

void Scene::setSceneName( const std::string& sceneName )
{
    name = sceneName;
}

const std::vector<SceneNode*>& Scene::getSceneNodes() const
{
    return sceneNodes;
}

const std::string& Scene::getSceneName() const
{
    return name;
}
