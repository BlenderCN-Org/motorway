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

namespace
{
    void RecomputeLeftAndUpVectors( RectangleLightData& data )
    {
        // Recompute up and left vectors automatically based on the plane normal
        // NOTE Only rectangular shaped area lights need both vectors, tube lights
        // only need left vector (because of its cylindrical shape)
        constexpr glm::vec3 RIGHT_DIRECTION = { 0.0f, 0.0f, 1.0f };
        constexpr glm::vec3 LEFT_DIRECTION = { 0.0f, 0.0f, -1.0f };

        glm::vec3 rightVector = glm::cross( data.planeNormal, RIGHT_DIRECTION );
        glm::vec3 leftVector = glm::cross( data.planeNormal, LEFT_DIRECTION );
        glm::vec3 upVector = glm::cross( rightVector, data.planeNormal );

        if ( glm::length( leftVector ) != 0.0f ) {
            data.leftVector = glm::normalize( leftVector );
        } else {
            data.leftVector = LEFT_DIRECTION;
        }

        if ( glm::length( upVector ) != 0.0f ) {
            data.upVector = glm::normalize( upVector );
        } else {
            data.upVector = glm::vec3( 0, 0, -1 );
        }
    }
}

struct RectangleLightSceneNode : public SceneNode 
{
    virtual RectangleLightSceneNode* clone( RenderableEntityManager* renderableEntityManager )
    {
        // Copy light and register a new instance in the renderable entity manager
        auto copiedLight = renderableEntityManager->createRectangleLight( std::move( *light ) );

        // Do a regular node copy and assign the copied light instance
        auto copiedNodeInstace = new RectangleLightSceneNode( *this );
        copiedNodeInstace->light = copiedLight;

#if FLAN_DEVBUILD
        // Copy editor preferences
        copiedNodeInstace->activeColorMode = activeColorMode;
#endif

        return copiedNodeInstace;
    }

    static constexpr fnStringHash_t Hashcode = FLAN_STRING_HASH( "RectangleLightSceneNode" );

    RectangleLight* light;
    flan::editor::eColorMode    activeColorMode;

    RectangleLightSceneNode( RectangleLight* nodeLight, const std::string& name = "Rectangle Light" )
        : SceneNode( name )
        , light( nodeLight )
    {
        canCollectRenderKeys = true;

#if FLAN_DEVBUILD
        activeColorMode = flan::editor::eColorMode::COLOR_MODE_SRGB;
#endif
    }

    RectangleLightSceneNode( RectangleLightSceneNode& node )
        : SceneNode( node )
    {

    }

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

        light = renderableEntityManager->createRectangleLight( {} );
        light->restore( stream );
    }

#if FLAN_DEVBUILD
    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime )
    {
        SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );

        auto& lightData = light->getLightData();

        ImGui::LabelText( "##RectangleLightName", "Rectangle/Tube Light" );

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

        if ( ImGui::DragFloat3( "Plane Normal (world)", &lightData.planeNormal[0] ) ) {
            RecomputeLeftAndUpVectors( lightData );
        }

        ImGui::DragFloat( "Width", &lightData.width, 0.01f, 0.0f, 64.0f );
        ImGui::DragFloat( "Height", &lightData.height, 0.01f, 0.0f, 64.0f );
        ImGui::Checkbox( "Is Tube Light", ( bool* )&lightData.isTubeLight );

        flan::editor::PanelLuminousIntensity( lightData.lightPower );
        flan::editor::PanelColor( activeColorMode, lightData.colorRGB );
    }
#endif
};
