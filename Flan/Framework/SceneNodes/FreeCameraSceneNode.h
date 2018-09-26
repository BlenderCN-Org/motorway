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

#include <Framework/Cameras/FreeCamera.h>
#include "SceneNode.h"
#include <Graphics/DrawCommandBuilder.h>

struct FreeCameraSceneNode : public SceneNode
{
    virtual FreeCameraSceneNode* clone( RenderableEntityManager* renderableEntityManager )
    {
        return new FreeCameraSceneNode( *this );
    }

    static constexpr fnStringHash_t Hashcode = FLAN_STRING_HASH( "FreeCameraSceneNode" );

    FreeCamera* camera;
    bool enabled;

    FreeCameraSceneNode( FreeCamera* nodeCamera, const std::string& name = "FreeCamera" )
        : SceneNode( name )
        , camera( nodeCamera )
        , enabled( false )
    {
        canUpdate = true;
        canCollectRenderKeys = true;
    }

    FreeCameraSceneNode( FreeCameraSceneNode& node )
        : SceneNode( node )
    {

    }

    virtual void update( const float frameTime ) override
    {
        camera->Update( frameTime );
    }

    virtual void collectRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        if ( enabled )
            drawCommandBuilder->addCamera( camera );
    }

    virtual void serialize( FileSystemObject* stream, const fnStringHash_t sceneNodeHashcode ) override
    {
        SceneNode::serialize( stream, Hashcode );
        camera->Serialize( stream );
    }

    virtual void deserialize( FileSystemObject* stream, GraphicsAssetManager* graphicsAssetManager, RenderableEntityManager* renderableEntityManager )  override
    {
        SceneNode::deserialize( stream, graphicsAssetManager, renderableEntityManager );

        // TODO Centralize camera instances
        if ( camera == nullptr ) {
            camera = new FreeCamera();
        }

        camera->Deserialize( stream );
    }

#if FLAN_DEVBUILD
    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime ) override
    {
        SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );
        camera->DrawInEditor( frameTime );
    }
#endif
};
