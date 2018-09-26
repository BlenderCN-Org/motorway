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

#include <Core/SerializationHelpers.h>

#include <Physics/RigidBody.h>

#if FLAN_DEVBUILD
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <glm/gtx/matrix_decompose.hpp>
#endif

class DrawCommandBuilder;
class GraphicsAssetManager;
class RenderableEntityManager;

struct SceneNode
{
    std::string                     name;
    fnStringHash_t                  hashcode;
    SceneNode*                      parent;
    std::vector<SceneNode*>         children;
    Transform                       transform;
    RigidBody*                      rigidBody;

    bool                            isDirty;

    // Node capabilities
    // Toggle one of these flag to call the associated function
    // each frame (you still need to override it)
    bool                            canCollectRenderKeys;
    bool                            canUpdate;
    bool                            canBeIntersected;
    bool                            canCollectDebugRenderKeys;
    bool                            canDrawInEditor;

    SceneNode( const std::string& nodeName = "Node" )
        : name( nodeName )
        , hashcode( flan::core::CRC32( name ) )
        , parent( nullptr )
        , isDirty( false )
        , transform{}
        , canCollectRenderKeys( false )
        , canUpdate( false )
        , canBeIntersected( false )
        , canCollectDebugRenderKeys( false )
        , rigidBody( nullptr )
    {
        name.resize( 256 );
    }

    SceneNode( SceneNode& node )
        : name( node.name )
        , hashcode( node.hashcode )
        , parent( node.parent )
        , isDirty( false )
        , transform( node.transform )
        , canCollectRenderKeys( node.canCollectRenderKeys )
        , canUpdate( node.canUpdate )
        , canBeIntersected( node.canBeIntersected )
        , canCollectDebugRenderKeys( node.canCollectDebugRenderKeys )
        , rigidBody( node.rigidBody )
    {

    }

    SceneNode* findChildByHashcode( const fnStringHash_t nodeHashcode )
    {
        // TODO Precompute hash/node map to avoid a vector lookup?
        for ( auto child : children ) {
            if ( child->hashcode == nodeHashcode ) {
                return child;
            }
        }

        return nullptr;
    }

    virtual SceneNode* clone( RenderableEntityManager* renderableEntityManager )
    {
        return new SceneNode( *this );
    }

#if FLAN_DEVBUILD
    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime )
    {
        FLAN_IMPORT_VAR_PTR( dev_IsInputText, bool )

        if ( ImGui::InputText( "Name", &name[0], 256, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue ) ) {
            *dev_IsInputText = !*dev_IsInputText;

            hashcode = flan::core::CRC32( name );
        }

        if ( ImGui::IsItemClicked() ) {
            *dev_IsInputText = !*dev_IsInputText;
        }

        transform.drawInEditor( frameTime, transactionHandler );
    }
#endif

    virtual void remove( RenderableEntityManager* renderableEntityManager )
    {

    }

    virtual void update( const float frameTime )
    {

    }

    virtual void collectRenderKeys( DrawCommandBuilder* drawCommandBuilder )
    {

    }

    virtual bool intersect( const Ray& ray, float& hitDistance ) const
    {
        return false;
    }

    virtual void collectDebugRenderKeys( DrawCommandBuilder* drawCommandBuilder )
    {

    }

    virtual void serialize( FileSystemObject* stream, const fnStringHash_t sceneNodeHashcode = 0x0 )
    {
        stream->write( sceneNodeHashcode );
        stream->write<int32_t>( ( parent != nullptr ) ? parent->hashcode : 0 );

        stream->writeString( name.c_str() );
        stream->write<uint8_t>( 0x0 );

        stream->write<uint8_t>( ( rigidBody != nullptr ) ? 1 : 0 );
        stream->write<uint8_t>( 0x0 ); // RESERVED       
        stream->write<uint8_t>( 0x0 ); // RESERVED
        stream->write<uint8_t>( 0x0 ); // RESERVED

        transform.serialize( stream );

       if ( rigidBody != nullptr ) {
           rigidBody->serialize( stream );
        }
    }

    virtual void deserialize( FileSystemObject* stream, GraphicsAssetManager* graphicsAssetManager, RenderableEntityManager* renderableEntityManager )
    {
        uint8_t hasRigidBody = 0, reserved1, reserved2, reserved3;
        FLAN_DESERIALIZE_VARIABLE( stream, hasRigidBody );

        // RESERVED
        FLAN_DESERIALIZE_VARIABLE( stream, reserved1 );
        FLAN_DESERIALIZE_VARIABLE( stream, reserved2 );
        FLAN_DESERIALIZE_VARIABLE( stream, reserved3 );

        transform.deserialize( stream );

        if ( hasRigidBody == 1 ) {
            rigidBody = new RigidBody( 0 );
            rigidBody->deserialize( stream );
        }
    }
};

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
