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

        static ImGuizmo::MODE mCurrentGizmoMode( ImGuizmo::WORLD );
        static bool useSnap = false;
        static int activeManipulationMode = 0;
        static float snap[3] = { 1.f, 1.f, 1.f };

        if ( ImGui::RadioButton( "Local", mCurrentGizmoMode == ImGuizmo::LOCAL ) )
            mCurrentGizmoMode = ImGuizmo::LOCAL;

        ImGui::SameLine();

        if ( ImGui::RadioButton( "World", mCurrentGizmoMode == ImGuizmo::WORLD ) )
            mCurrentGizmoMode = ImGuizmo::WORLD;

        ImGui::Checkbox( "", &useSnap );
        ImGui::SameLine();
        ImGui::InputFloat( "Snap", &snap[0] );

        ImGui::RadioButton( "Translate", &activeManipulationMode, 0 );
        ImGui::SameLine();
        ImGui::RadioButton( "Rotation", &activeManipulationMode, 1 );
        ImGui::SameLine();
        ImGui::RadioButton( "Scale", &activeManipulationMode, 2 );

        FLAN_IMPORT_VAR_PTR( dev_GuizmoViewMatrix, float* )
        FLAN_IMPORT_VAR_PTR( dev_GuizmoProjMatrix, float* )
            
        glm::mat4 modelMatrix( 1 );
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect( 0, 0, io.DisplaySize.x, io.DisplaySize.y );
        ImGuizmo::Manipulate( *dev_GuizmoViewMatrix, *dev_GuizmoProjMatrix, static_cast< ImGuizmo::OPERATION >( activeManipulationMode ), mCurrentGizmoMode, ( float* )&modelMatrix[0][0], NULL, useSnap ? &snap[0] : NULL );

        glm::vec3 scaleDecomposed;
        glm::quat rotationDecomposed;
        glm::vec3 skewDecomposed;
        glm::vec4 perspectiveDecomposed;
        glm::decompose( modelMatrix, scaleDecomposed, rotationDecomposed, lightData.worldPosition, skewDecomposed, perspectiveDecomposed );

        lightData.radius = glm::min( 0.01f, scaleDecomposed.x );

        ImGui::DragFloat3( "Position (world)", &lightData.worldPosition[0] );
        ImGui::DragFloat3( "Plane Normal (world)", &lightData.planeNormal[0] );
        ImGui::DragFloat( "Radius", &lightData.radius, 0.01f, 0.01f, 64.0f );

        flan::editor::PanelLuminousIntensity( lightData.lightPower );
        flan::editor::PanelColor( activeColorMode, lightData.colorRGB );
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

    /*virtual void DrawInWorld( WorldRenderer* WorldRenderer, bool isSelected = false ) override
    {
        DrawCommand& drawCommand = WorldRenderer->AddCircleWireframeDrawCall();
        drawCommand.SortKey.MeshHashcode = PA_STRING_HASH( "DBG_CIRCLE" );
        drawCommand.ModelMatrix = light->GetModelMatrix();
    }

    virtual bool Intersects( const Ray& ray, float& hitDistance ) const override
    {
        const auto& lightData = light->GetLightData();

        auto lightRadius = GetLightScaledRadius();
        return pa::maths::RayDiskIntersectionTest( lightData.WorldPosition, lightRadius, lightData.PlaneNormal, ray, hitDistance );
    }

    virtual void ComputeBounds( const glm::vec3& planeNormal, float& nearBound, float& farBound ) const override
    {
        const auto& lightPosition = light->GetLightData().WorldPosition;
        auto lightRadius = GetLightScaledRadius();

        auto minPoint = lightPosition - lightRadius;
        auto maxPoint = lightPosition + lightRadius;

        float d = glm::dot( planeNormal, minPoint );
        nearBound = ( d < nearBound ) ? d : nearBound;

        d = glm::dot( planeNormal, maxPoint );
        farBound = ( d > farBound ) ? d : nearBound;
    }*/
};
