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

#include <Graphics/RenderableEntityManager.h>
#include <Graphics/DrawCommandBuilder.h>

#include "Light.h"

#include <Core/Maths/Disk.h>

struct DiskLightSceneNode : public SceneNode 
{
    virtual DiskLightSceneNode* clone( RenderableEntityManager* renderableEntityManager )
    {
        // Copy light and register a new instance in the renderable entity manager
        auto copiedLight = renderableEntityManager->createDiskLight( std::move( *light ) );

        // Do a regular node copy and assign the copied light instance
        auto copiedNodeInstace = new DiskLightSceneNode( *this );
        copiedNodeInstace->light = copiedLight;

#if FLAN_DEVBUILD
        // Copy editor preferences
        copiedNodeInstace->activeColorMode = activeColorMode;
#endif

        return copiedNodeInstace;
    }

    static constexpr fnStringHash_t Hashcode = FLAN_STRING_HASH( "DiskLightSceneNode" );

    DiskLight* light;

private:
    float getLightScaledRadius() const
    {
        const auto& lightData = light->getLightData();
        return ( lightData.radius * transform.getWorldBiggestScale() );
    }

public:
    DiskLightSceneNode( DiskLight* nodeLight, const std::string& name = "Disk Light" )
        : SceneNode( name )
        , light( nodeLight )
    {
        canCollectRenderKeys = true;
        canDrawInEditor = true;
        canCollectDebugRenderKeys = true;
    }

    DiskLightSceneNode( DiskLightSceneNode& node )
        : SceneNode( node )
    {

    }

#if FLAN_DEVBUILD
    flan::editor::eColorMode activeColorMode;

    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime )
    {
        SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );

        auto& lightData = light->getLightData();

        ImGui::LabelText( "##DiskLightName", "Disk Light" );

        if ( ImGui::DragFloat3( "Position (world)", &lightData.worldPosition[0] ) ) {
            transform.setLocalTranslation( lightData.worldPosition );
        } else {
            lightData.worldPosition = transform.getWorldTranslation();
        }

        if ( ImGui::DragFloat( "Radius", &lightData.radius, 0.01f, 0.01f, 64.0f ) ) {
            transform.setLocalScale( glm::vec3( lightData.radius ) );
        } else {
            lightData.radius = transform.getWorldBiggestScale();
        }

        ImGui::DragFloat3( "Position (world)", &lightData.worldPosition[0] );
        ImGui::DragFloat3( "Plane Normal (world)", &lightData.planeNormal[0] );
        ImGui::DragFloat( "Radius", &lightData.radius, 0.01f, 0.01f, 64.0f );

        flan::editor::PanelLuminousIntensity( lightData.lightPower );
        flan::editor::PanelColor( activeColorMode, lightData.colorRGB );
    }

    virtual void collectDebugRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        const auto& lightData = light->getLightData();
        glm::quat rotationMatrix = glm::rotate( glm::quat( 1.0f, 0.0f, 0.0f, 0.0f ), glm::radians( 90.0f * lightData.planeNormal.x ), glm::vec3( 1, 0, 0 ) );
        rotationMatrix = glm::rotate( rotationMatrix, glm::radians( 90.0f * lightData.planeNormal.y ), glm::vec3( 0, 1, 0 ) );
        rotationMatrix = glm::rotate( rotationMatrix, glm::radians( 90.0f * lightData.planeNormal.z ), glm::vec3( 0, 0, 1 ) );

        drawCommandBuilder->addWireframeCircle( lightData.worldPosition, lightData.radius, rotationMatrix );
    }
#endif

    virtual void remove( RenderableEntityManager* renderableEntityManager ) override
    {
        if ( light != nullptr ) {
            renderableEntityManager->removeEntity( light->getRenderKey() );
        }
    }

    virtual void collectRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        drawCommandBuilder->addEntityToUpdate( light->getRenderKey() );
    }

    virtual void serialize( FileSystemObject* stream, const fnStringHash_t sceneNodeHashcode ) override
    {
        SceneNode::serialize( stream, Hashcode );
        light->save( stream );
    }

    virtual void deserialize( FileSystemObject* stream, GraphicsAssetManager* graphicsAssetManager, RenderableEntityManager* renderableEntityManager )  override
    {
        SceneNode::deserialize( stream, graphicsAssetManager, renderableEntityManager );

        light = renderableEntityManager->createDiskLight( {} );
        light->restore( stream );
    }
};
