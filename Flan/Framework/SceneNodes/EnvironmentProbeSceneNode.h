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
#include <Graphics/EnvironmentProbe.h>
#include <Graphics/RenderableEntityManager.h>
#include <Graphics/DrawCommandBuilder.h>

struct EnvironmentProbeSceneNode : public SceneNode
{
private:
    glm::mat4   modelMatrix;
    glm::vec3   previousProbePosition;

    uint32_t    lastCaptureTicksCount;

public:
    virtual EnvironmentProbeSceneNode* clone( RenderableEntityManager* renderableEntityManager )
    {
        // Copy light and register a new instance in the renderable entity manager
        EnvironmentProbe* copiedProbe = nullptr;
        if ( Probe->IsFallbackProbe ) {
            copiedProbe = renderableEntityManager->createGlobalEnvironmentProbe();
        } else {
            copiedProbe = renderableEntityManager->createLocalEnvironmentProbe();
        }

        if ( Probe->IsDynamic ) {
            copiedProbe->IsDynamic = true;
            copiedProbe->CaptureFrequency = Probe->CaptureFrequency;
        }

        // Do a regular node copy and assign the copied probe instance
        auto copiedNodeInstace = new EnvironmentProbeSceneNode( *this );  
        copiedNodeInstace->Probe = copiedProbe;

        return copiedNodeInstace;
    }

    static constexpr fnStringHash_t Hashcode = FLAN_STRING_HASH( "EnvironmentProbeSceneNode" );

    EnvironmentProbe* Probe;

    EnvironmentProbeSceneNode( EnvironmentProbe* nodeProbe, const std::string& name = "Light Probe" )
        : SceneNode( name )
        , modelMatrix( 1.0f )
        , previousProbePosition( 0, 0, 0 )
        , lastCaptureTicksCount( 0 )
        , Probe( nodeProbe )
    {
        canCollectRenderKeys = true;
        canCollectDebugRenderKeys = true;
    }

    EnvironmentProbeSceneNode( EnvironmentProbeSceneNode& node )
        : SceneNode( node )
    {

    }

    virtual void remove( RenderableEntityManager* renderableEntityManager ) override
    {
        if ( Probe != nullptr ) {
            renderableEntityManager->removeEntity( Probe->RenderKey );
        }
    }

#if FLAN_DEVBUILD
    virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime ) override
    {
        SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );

        if ( ImGui::Button( "Force Probe Capture" ) ) {
            Probe->IsCaptured = false;
        }

        ImGui::Checkbox( "Is Fallback Probe", &Probe->IsFallbackProbe );
        ImGui::Checkbox( "Is Dynamic", &Probe->IsDynamic );

        if ( Probe->IsDynamic ) {
            ImGui::InputInt( "Update Frequency (in frames)", ( int* )&Probe->CaptureFrequency );
        }
    }

    virtual void collectDebugRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        drawCommandBuilder->addWireframeAABB( Probe->Sphere.center, glm::vec3( Probe->Sphere.radius ) );
    }
#endif

    virtual void collectRenderKeys( DrawCommandBuilder* drawCommandBuilder ) override
    {
        if ( Probe->IsDynamic ) {
            // Since it relies on render ticks (and not logic ticks), this is the easiest and cheapest way to check whether or not 
            // the probe needs to be recaptured
            lastCaptureTicksCount++;
        
            if ( lastCaptureTicksCount >= Probe->CaptureFrequency ) {
                Probe->IsCaptured = false;
                lastCaptureTicksCount = 0;
            }
        }

        // Update probe bounding sphere with node transform infos
        modelMatrix = *transform.getWorldModelMatrix();

        Probe->InverseModelMatrix = glm::transpose( glm::inverse( modelMatrix ) );

        auto probePosition = transform.getWorldTranslation();

        if ( !Probe->IsCaptured 
          || ( previousProbePosition != probePosition && !transform.IsManipulating() ) ) {
            Probe->Sphere.center = probePosition;

            Probe->Sphere.radius = transform.getWorldBiggestScale();
            previousProbePosition = probePosition;

            drawCommandBuilder->addEntityToUpdate( Probe->RenderKey );
            drawCommandBuilder->addEnvProbeToCapture( Probe );
        }
    }

    virtual void serialize( FileSystemObject* stream, const fnStringHash_t sceneNodeHashcode ) override
    {
        SceneNode::serialize( stream, Hashcode );
        stream->write( Probe->IsFallbackProbe );
        stream->write( ( Probe->IsCaptured ) ? Probe->ProbeIndex : 0xFFFFFFFF );
        stream->write( Probe->IsDynamic );
        stream->write( Probe->CaptureFrequency );
        stream->write( Probe->Sphere.center );
        stream->write( Probe->Sphere.radius );
    }

    virtual void deserialize( FileSystemObject* stream, GraphicsAssetManager* graphicsAssetManager, RenderableEntityManager* renderableEntityManager ) override
    {
        SceneNode::deserialize( stream, graphicsAssetManager, renderableEntityManager );

        bool isFallbackProbe = false;
        stream->read( isFallbackProbe );

        Probe = ( isFallbackProbe )
            ? renderableEntityManager->createGlobalEnvironmentProbe()
            : renderableEntityManager->createLocalEnvironmentProbe();

        stream->read( Probe->ProbeIndex );

        // If the probe has a serialized index, its data are already saved to disk (no need to recapture at runtime)
        if ( Probe->ProbeIndex != 0xFFFFFFFF ) {
            Probe->IsCaptured = true;
        }

        stream->read( Probe->IsDynamic );
        stream->read( Probe->CaptureFrequency );
        stream->read( Probe->Sphere.center );
        stream->read( Probe->Sphere.radius );

        transform.setLocalTranslation( Probe->Sphere.center );
        transform.setLocalScale( glm::vec3( Probe->Sphere.radius ) );
    }

    //virtual void drawInEditor( GraphicsAssetManager* graphicsAssetManager, TransactionHandler* transactionHandler, const float frameTime ) override
    //{
    //    SceneNode::drawInEditor( graphicsAssetManager, transactionHandler, frameTime );

    //    if ( ImGui::Button( "Force Probe Capture" ) ) {
    //        Probe->IsCaptured = false;
    //    }

    //    ImGui::Checkbox( "Is Fallback Probe", &Probe->IsFallbackProbe );
    //    ImGui::Checkbox( "Is Dynamic", &Probe->IsDynamic );

    //    if ( Probe->IsDynamic ) {
    //        ImGui::InputInt( "Update Frequency (in frames)", ( int* )&Probe->CaptureFrequency);
    //    }
    //}
};
