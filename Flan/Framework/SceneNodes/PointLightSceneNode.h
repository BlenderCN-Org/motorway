/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

struct PointLightSceneNode : public SceneNode 
{
    virtual PointLightSceneNode* clone( RenderableEntityManager* renderableEntityManager ) override
    {
        // Copy light and register a new instance in the renderable entity manager
        auto copiedLight = renderableEntityManager->createPointLight( std::move( *light ) );

        // Do a regular node copy and assign the copied light instance
        auto copiedNodeInstace = new PointLightSceneNode( *this );
        copiedNodeInstace->light = copiedLight;

#if FLAN_DEVBUILD
        // Copy editor preferences
        copiedNodeInstace->activeColorMode = activeColorMode;
#endif

        return copiedNodeInstace;
    }

    static constexpr fnStringHash_t Hashcode = FLAN_STRING_HASH( "PointLightSceneNode" );

    PointLight* light;
    flan::editor::eColorMode activeColorMode;

    PointLightSceneNode( PointLight* nodeLight, const std::string& name = "Point Light" )
        : SceneNode( name )
        , light( nodeLight )
    {
        canCollectRenderKeys = true;
        canBeIntersected = true;

#if FLAN_DEVBUILD
        activeColorMode = flan::editor::COLOR_MODE_SRGB;
        canCollectDebugRenderKeys = true;
#endif
    }

    PointLightSceneNode( PointLightSceneNode& node )
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

#if FLAN_DEVBUILD
    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime )
    {
        SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );

        auto& lightData = light->getLightData();

        ImGui::LabelText( "##PointLightName", "Point Light" );

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

        flan::editor::PanelLuminousIntensity( lightData.lightPower );
        flan::editor::PanelColor( activeColorMode, lightData.colorRGB );
    }

    virtual void collectDebugRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        const auto& lightData = light->getLightData();
        drawCommandBuilder->addWireframeSphere( lightData.worldPosition, lightData.radius, glm::vec4( lightData.colorRGB, 1.0f ) );
    }
#endif

    virtual void serialize( FileSystemObject* stream, const fnStringHash_t sceneNodeHashcode ) override
    {
        SceneNode::serialize( stream, Hashcode );
        light->save( stream );
    }

    virtual void deserialize( FileSystemObject* stream, GraphicsAssetManager* graphicsAssetManager, RenderableEntityManager* renderableEntityManager )  override
    {
        SceneNode::deserialize( stream, graphicsAssetManager, renderableEntityManager );

        light = renderableEntityManager->createPointLight( {} );
        light->restore( stream );
    }

    virtual bool intersect( const Ray& ray, float& hitDistance ) const override
    {
        auto nodeScale = transform.getWorldBiggestScale();
        auto nodeTranslation = transform.getWorldTranslation();

        const auto& lightData = light->getLightData();

        auto lightRadiusScaled = ( nodeScale * lightData.radius );

        BoundingSphere sphere;
        flan::core::CreateSphere( sphere, nodeTranslation + lightData.worldPosition, lightRadiusScaled * 4.0f );

        return flan::core::RaySphereIntersectionTest( sphere, ray, hitDistance );
    }
};
