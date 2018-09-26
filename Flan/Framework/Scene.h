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

#include <vector>
#include <string>

#include <Core/Maths/Transform.h>
#include <Core/Maths/Ray.h>

#include <Core/Hashing/CRC32.h>
#include <Core/Factory.h>

#include "SceneNodes/MeshSceneNode.h"
#include "SceneNodes/ModelSceneNode.h"
#include "SceneNodes/FreeCameraSceneNode.h"
#include "SceneNodes/DirectionalLightSceneNode.h"
#include "SceneNodes/PointLightSceneNode.h"
#include "SceneNodes/SpotLightSceneNode.h"
#include "SceneNodes/SphereLightSceneNode.h"
#include "SceneNodes/DiskLightSceneNode.h"
#include "SceneNodes/RectangleLightSceneNode.h"
#include "SceneNodes/EnvironmentProbeSceneNode.h"
#include "SceneNodes/FirstPersonCameraSceneNode.h"
#include "SceneNodes/LookAtCameraSceneNode.h"
#include "SceneNodes/GameObjectNode.h"

#include "Mesh.h"
#include "Light.h"

class DrawCommandBuilder;

class Scene
{
#define FLAN_FACTORY_REGISTER_ENTITY( factoryType, entityToRegister )\
static auto SceneNodeBuilder##entityToRegister = [=]( Scene* scene, fnStringHash_t parentHashcode, std::string& nodeName ) { return scene->create##entityToRegister( nullptr, nodeName, scene->findNodeByHashcode( parentHashcode ) ); };\
static bool SceneNodeIsRegistered##entityToRegister = Factory<factoryType, Scene*, fnStringHash_t, std::string&>::registerComponent( entityToRegister##SceneNode::Hashcode, SceneNodeBuilder##entityToRegister );

#define FLAN_SCENE_CREATE_NODE_FUNC( type )\
auto create##type( type* type##Instance, const std::string& nodeName = std::string( #type ), SceneNode* parent = nullptr ) -> SceneNode*\
{\
sceneNodes.push_back( new ( type##SceneNode )( type##Instance, nodeName ) );\
auto& pushedNode = sceneNodes.back();\
\
buildNode( pushedNode, parent );\
\
return pushedNode;\
}

public:
                                    Scene( const std::string& sceneName = "Default Scene" );
                                    Scene( Scene& scene ) = default;
                                    Scene& operator = ( Scene& scene ) = default;
                                    ~Scene();

    SceneNode*                      createEmptyNode( const std::string& nodeName = "Empty Node", SceneNode* parent = nullptr );

    FLAN_SCENE_CREATE_NODE_FUNC( Mesh )
    FLAN_SCENE_CREATE_NODE_FUNC( Model )
    FLAN_SCENE_CREATE_NODE_FUNC( FreeCamera )
    FLAN_SCENE_CREATE_NODE_FUNC( DirectionalLight )
    FLAN_SCENE_CREATE_NODE_FUNC( PointLight )
    FLAN_SCENE_CREATE_NODE_FUNC( SpotLight )
    FLAN_SCENE_CREATE_NODE_FUNC( SphereLight )
    FLAN_SCENE_CREATE_NODE_FUNC( DiskLight )
    FLAN_SCENE_CREATE_NODE_FUNC( RectangleLight )
    FLAN_SCENE_CREATE_NODE_FUNC( EnvironmentProbe )
    FLAN_SCENE_CREATE_NODE_FUNC( FirstPersonCamera )
    FLAN_SCENE_CREATE_NODE_FUNC( LookAtCamera )
    FLAN_SCENE_CREATE_NODE_FUNC( GameObject )
        
    SceneNode*                      addNode( SceneNode* node );
    void                            clearNodes();
    bool                            removeNode( const fnStringHash_t nodeHashcode );
    SceneNode*                      findNodeByHashcode( const fnStringHash_t nodeHashcode ) const;
    SceneNode*                      intersect( const Ray& ray ) const;

    void                            update( const float frameTime );
    void                            collectRenderKeys( DrawCommandBuilder* drawCmdBuilder );

    void                            setSceneName( const std::string& sceneName );
    const std::string&              getSceneName() const;
    const std::vector<SceneNode*>&  getSceneNodes() const;

private:
    std::vector<SceneNode*>             sceneNodes;
    std::string                         name;
    std::map<fnStringHash_t, size_t>    nodeHelper; // <hashcode / vector> index map helper

    int64_t                                canCollectRenderKeys[8192];
    int64_t                                canUpdate[8192];
    //bool                            canBeIntersected[8192];
    //bool                            canCollectDebugRenderKeys[8192];
    //bool                            canDrawInEditor[8192];

private:
    void                buildNode( SceneNode* node, SceneNode* parent = nullptr );
};

FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, Mesh )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, Model )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, FreeCamera )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, DirectionalLight )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, PointLight )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, SpotLight )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, SphereLight )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, DiskLight )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, RectangleLight )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, EnvironmentProbe )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, FirstPersonCamera )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, LookAtCamera )
FLAN_FACTORY_REGISTER_ENTITY( SceneNode*, GameObject )
