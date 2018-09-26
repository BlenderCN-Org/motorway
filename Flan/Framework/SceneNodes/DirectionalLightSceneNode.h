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

#include <Core/ColormetryHelpers.h>
#include <Core/Maths/CoordinatesSystemHelpers.h>

#include <Graphics/RenderableEntityManager.h>
#include <Graphics/DrawCommandBuilder.h>

#include "Light.h"

struct DirectionalLightSceneNode : public SceneNode 
{
    static constexpr fnStringHash_t Hashcode = FLAN_STRING_HASH( "DirectionalLightSceneNode" );

    DirectionalLight* light;

    DirectionalLightSceneNode( DirectionalLight* nodeLight, const std::string& name = "Directional Light" )
        : SceneNode( name )
        , light( nodeLight )
    {
        canCollectRenderKeys = true;
        canDrawInEditor = true;
    }

#if FLAN_DEVBUILD
    flan::editor::eColorMode activeColorMode;

    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime )
    {
        SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );

        auto& lightData = light->getLightData();

        ImGui::LabelText( "##DirectionalLightName", "Directional Light" );

        ImGui::Checkbox( "Enable Shadow", &lightData.enableShadow );
        ImGui::SameLine();
        ImGui::Checkbox( "Acts as Sun", &lightData.isSunLight );

        flan::editor::PanelLuminousIntensity( lightData.intensityInLux );
        flan::editor::PanelColor( activeColorMode, lightData.colorRGB );

        ImGui::DragFloat( "Angular Radius", &lightData.angularRadius, 0.00001f, 0.0f, 1.0f );

        const float solidAngle = ( 2.0f * glm::pi<float>() ) * ( 1.0f - cos( lightData.angularRadius ) );

        lightData.illuminanceInLux = lightData.intensityInLux * solidAngle;

        ImGui::DragFloat( "Spherical Coordinate Theta", &lightData.sphericalCoordinates.x, 0.01f, -1.0f, 1.0f );
        ImGui::DragFloat( "Spherical Coordinate Gamma", &lightData.sphericalCoordinates.y, 0.01f, -1.0f, 1.0f );

        lightData.direction = flan::core::SphericalToCarthesianCoordinates( lightData.sphericalCoordinates.x, lightData.sphericalCoordinates.y );

        if ( lightData.isSunLight ) {
            // Angles are sent to the atmosphere render module
            FLAN_IMPORT_VAR_PTR( dev_SunVerticalAngle, float )
            FLAN_IMPORT_VAR_PTR( dev_SunHorizontalAngle, float )

            *dev_SunVerticalAngle = lightData.sphericalCoordinates.x;
            *dev_SunHorizontalAngle = lightData.sphericalCoordinates.y;
        }
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

        DirectionalLight dirLight;
        dirLight.restore( stream );

        light = renderableEntityManager->createDirectionalLight( std::move( dirLight ) );

        auto& lightData = light->getLightData();
        if ( lightData.isSunLight ) {
            // Angles are sent to the atmosphere render module
            FLAN_IMPORT_VAR_PTR( dev_SunVerticalAngle, float )
            FLAN_IMPORT_VAR_PTR( dev_SunHorizontalAngle, float )

            *dev_SunVerticalAngle = lightData.sphericalCoordinates.x;
            *dev_SunHorizontalAngle = lightData.sphericalCoordinates.y;
        }
    }
};
