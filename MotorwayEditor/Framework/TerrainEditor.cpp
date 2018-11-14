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
#include <Shared.h>
#include "TerrainEditor.h"

#include <AppShared.h>

#include <Framework/SceneNodes/SceneNode.h>
#include <Framework/TransactionHandler/TransactionHandler.h>
#include <Framework/TransactionHandler/SceneNodeCopyCommand.h>
#include <Framework/TransactionHandler/SceneNodeDeleteCommand.h>

#include <FileSystem/FileSystemObjectNative.h>

#include <Io/Collider.h>

#include <imgui/imgui.h>

static int g_TerrainEditorEditionMode = 0; // TODO Make it enum!
static int g_TerrainEditorMode = 0; // TODO Make it enum!
static int g_TerrainEditorBrushType = 0; // TODO Make it enum! Square Default
static int g_TerrainEditionMaterialIndex1 = 0;
static int g_TerrainEditionMaterialIndex2 = 0;
static float g_TerrainEditionMaterialHardness1 = 1.0f;
static float g_TerrainEditionMaterialHardness2 = 1.0f;
static float g_TerrainEditorEditionHeight = 0.01f;
static float g_TerrainEditorEditionHardness = 1.0f;

FLAN_DEV_VAR( g_TerrainEditorEditionRadius, "TerrainEd Brush Radius (World Space)", 8, int )
FLAN_DEV_VAR( dev_TerrainMousePosition, "TerrainEd Mouse Position (World Space Position)", {}, glm::vec3 )

void flan::framework::DisplayTerrainEditor()
{
    ImGui::RadioButton( "Sculpting", &g_TerrainEditorMode, 0 );
    ImGui::SameLine();
    ImGui::RadioButton( "Paiting", &g_TerrainEditorMode, 1 );

    if ( g_TerrainEditorMode == 0 ) {
        ImGui::RadioButton( "Raise", &g_TerrainEditorEditionMode, 0 );
        ImGui::SameLine();
        ImGui::RadioButton( "Lower", &g_TerrainEditorEditionMode, 1 );
        ImGui::SameLine();
        ImGui::RadioButton( "Smooth", &g_TerrainEditorEditionMode, 2 );
    }

    if ( ImGui::TreeNode( "Brush Settings" ) ) {
        ImGui::SliderInt( "Radius", &g_TerrainEditorEditionRadius, 1, 256 );

        if ( g_TerrainEditorMode == 0 ) {
            ImGui::DragFloat( "Value", &g_TerrainEditorEditionHeight, 0.00001f, 1.0f );
        } else if ( g_TerrainEditorMode == 1 ) {
            // TODO User friendly!
            //  Selectable material (links to the biome editor?)
            //  Visual feedback in the panel (albedo texture?)
            ImGui::DragInt( "Base Material Index", &g_TerrainEditionMaterialIndex1, 1.0f, 0, std::numeric_limits<uint16_t>::max() );
            ImGui::DragInt( "Overlayer Material Index", &g_TerrainEditionMaterialIndex2, 1.0f, 0, std::numeric_limits<uint16_t>::max() );
        }

        ImGui::DragFloat( "Hardness", &g_TerrainEditorEditionHardness, 0.00001f, 0.0f, 1.0f );

        ImGui::Text( "Shape" );
        ImGui::RadioButton( "Square", &g_TerrainEditorBrushType, 0 );
        ImGui::RadioButton( "Circle", &g_TerrainEditorBrushType, 1 );
        ImGui::TreePop();
    }
}

bool IsInRadius( const float rSquared, const int x, const int y, const int pointX, const int pointY )
{
    auto xSquared = ( pointX - x );
    xSquared *= xSquared;

    auto ySquared = ( pointY - y );
    ySquared *= ySquared;

    return ( xSquared + ySquared ) < rSquared;
}


void flan::framework::EditTerrain( const Ray& mousePickingRay, Terrain* terrain, CommandList* cmdList )
{
    float* const vertices = terrain->getHeightmapValues();

    float pickedHeight = -std::numeric_limits<float>::max();
    int pickedIndex = 0;
    glm::vec3 rayMarch = mousePickingRay.origin;
    int marchIteration = 0;
    while ( rayMarch.y > pickedHeight && marchIteration < 512 ) {
        pickedIndex = ( abs( int( rayMarch.x ) ) + abs( int( rayMarch.z ) ) * 512 );
        pickedHeight = vertices[pickedIndex];

        rayMarch += mousePickingRay.direction;
        marchIteration++;
    }

    dev_TerrainMousePosition = glm::vec3( rayMarch.x, pickedHeight, rayMarch.z );

    auto lowX = std::max( 0, abs( int( rayMarch.x ) ) - g_TerrainEditorEditionRadius );
    auto lowY = std::max( 0, abs( int( rayMarch.z ) ) - g_TerrainEditorEditionRadius );
    auto hiX = std::min( 512, abs( int( rayMarch.x ) ) + g_TerrainEditorEditionRadius );
    auto hiY = std::min( 512, abs( int( rayMarch.z ) ) + g_TerrainEditorEditionRadius );

    float k = static_cast<float>( g_TerrainEditorEditionRadius * g_TerrainEditorEditionRadius );

    if ( g_TerrainEditorMode == 0 ) {
        if ( g_TerrainEditorEditionMode == 2 ) {
            // Basic box filtering-ish smoothing
            const int sampleCount = ( hiY - lowY ) * ( hiX - lowX );

            float average = 0.0f;
            for ( int x = lowX; x < hiX; x++ ) {
                for ( int y = lowY; y < hiY; y++ ) {
                    int index = ( x + y * 512 );
                    average += vertices[index];
                }
            }

            average /= sampleCount;

            for ( int x = lowX; x < hiX; x++ ) {
                for ( int y = lowY; y < hiY; y++ ) {
                    int index = ( x + y * 512 );

                    if ( g_TerrainEditorBrushType == 1
                        && !IsInRadius( k, int( rayMarch.x ), int( rayMarch.z ), x, y ) ) {
                        continue;
                    }

                    terrain->setVertexHeight( index, average );
                }
            }
        } else {
            for ( int x = lowX; x < hiX; x++ ) {
                for ( int y = lowY; y < hiY; y++ ) {
                    // Attenuation factor
                    float distance = glm::length( glm::vec2( rayMarch.x, rayMarch.z ) - glm::vec2( x, y ) ) / k;

                    int index = ( x + y * 512 );
                    float& vertexToEdit = vertices[index];

                    if ( g_TerrainEditorBrushType == 1
                        && !IsInRadius( k, int( rayMarch.x ), int( rayMarch.z ), x, y ) ) {
                        continue;
                    } else {
                        // For square brush ignore distance attenuation
                        distance = 1.0f;
                    }

                    if ( g_TerrainEditorEditionMode == 0 )
                        vertexToEdit += ( g_TerrainEditorEditionHeight * g_TerrainEditorEditionHardness ) * distance;
                    else if ( g_TerrainEditorEditionMode == 1 )
                        vertexToEdit -= ( g_TerrainEditorEditionHeight * g_TerrainEditorEditionHardness ) * distance;

                    terrain->setVertexHeight( index, vertexToEdit );
                }
            }
        }

        terrain->uploadHeightmap( cmdList );
    } else {
        for ( int x = lowX; x < hiX; x++ ) {
            for ( int y = lowY; y < hiY; y++ ) {
                float distance = glm::length( glm::vec2( rayMarch.x, rayMarch.z ) - glm::vec2( x, y ) ) / k;
                int index = ( x + y * 512 ) * 4;

                if ( g_TerrainEditorBrushType == 1
                    && !IsInRadius( k, int( rayMarch.x ), int( rayMarch.z ), x, y ) ) {
                    continue;
                }

                terrain->setVertexMaterial( index, g_TerrainEditionMaterialIndex1, g_TerrainEditionMaterialIndex2, g_TerrainEditorEditionHardness );
            }
        }

        terrain->uploadSplatmap( cmdList );
    }
}
