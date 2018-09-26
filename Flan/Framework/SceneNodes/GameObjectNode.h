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

#include "SceneNode.h"

#include <Framework/GameObject.h>

struct GameObjectSceneNode : public SceneNode
{
    static constexpr fnStringHash_t Hashcode = FLAN_STRING_HASH( "GameObjectSceneNode" );

    GameObject* gameObject;

    GameObjectSceneNode( GameObject* gameObjectInstance, const std::string& name = "GameObject" )
        : SceneNode( name )
        , gameObject( gameObjectInstance )
    {
        canUpdate = true;
    }

    GameObjectSceneNode( GameObjectSceneNode& node )
        : SceneNode( node )
    {

    }

    virtual void update( const float frameTime ) override
    {
        gameObject->Update( frameTime );
    }

    virtual void serialize( FileSystemObject* stream, const fnStringHash_t sceneNodeHashcode ) override
    {
        SceneNode::serialize( stream, Hashcode );
        gameObject->Serialize( stream );
    }

    virtual void deserialize( FileSystemObject* stream, GraphicsAssetManager* graphicsAssetManager, RenderableEntityManager* renderableEntityManager )  override
    {
        SceneNode::deserialize( stream, graphicsAssetManager, renderableEntityManager );

        gameObject->Deserialize( stream );
    }

#if FLAN_DEVBUILD
    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime ) override
    {
        SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );
    }
#endif
};
