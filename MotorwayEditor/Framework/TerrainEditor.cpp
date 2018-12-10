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
#include <Framework/TransactionHandler/ImGuiEditCommand.h>
#include <Core/Allocators/LinearAllocator.h>

#include <FileSystem/FileSystemObjectNative.h>

#include <Io/Collider.h>

#if FLAN_D3D11
#include <Rendering/Direct3D11/Texture.h>
#elif FLAN_VULKAN
#include <Rendering/Vulkan/Texture.h>
#endif

#include <Core/PathHelpers.h>

#include <imgui/imgui.h>

static int g_TerrainEditorEditionMode = 0; // TODO Make it enum!
static int g_TerrainEditorMode = 0; // TODO Make it enum!
static int g_TerrainEditorBrushType = 1; // TODO Make it enum! Square Default
static int g_TerrainEditionMaterialIndex1 = 0;
static int g_TerrainEditionMaterialIndex2 = 0;
static Texture* g_TerrainEditionBaseMaterialBaseColor = nullptr;
static Texture* g_TerrainEditionBaseMaterialNormal = nullptr;
static float g_TerrainEditionMaterialHardness1 = 1.0f;
static float g_TerrainEditionMaterialHardness2 = 1.0f;
static float g_TerrainEditorEditionHeight = 0.01f;
static float g_TerrainEditorEditionHardness = 1.0f;
static glm::vec3 g_TerrainEditorGrassColor = glm::vec3( 0, 1, 0 );

FLAN_DEV_VAR( TerrainEditionHeightDelta, "", 0.0f, float )
FLAN_DEV_VAR( g_TerrainEditorEditionRadius, "TerrainEd Brush Radius (World Space)", 8, int )
FLAN_DEV_VAR( dev_TerrainMousePosition, "TerrainEd Mouse Position (World Space Position)", {}, glm::vec3 )
FLAN_DEV_VAR( g_RayMarch, "TerrainEd RayMouse Position (World Space Position)", glm::vec3( 0, 0, 0 ), glm::vec3 )

void DisplayTextureInput( const std::string& name, Texture*& outputTexture )
{
    // Input label
    ImVec2 texInfosSize = ImVec2( 318.0f, 64.0f );
    std::string imageInfos = "No texture selected...";

    ImGui::Text( name.c_str() );
    if ( outputTexture != nullptr ) {
        // Build Image Infos Text
        imageInfos.clear();

        const auto& textureName = outputTexture->getResourceName();
        const auto& textureDesc = outputTexture->getDescription();

        imageInfos += "Name: " + textureName + "\n";
        imageInfos += "Dimensions: " + std::to_string( textureDesc.width ) + "x"
            + std::to_string( textureDesc.height ) + "\n";
        imageInfos += "Mip Count: " + std::to_string( textureDesc.mipCount );


#if FLAN_D3D11
        auto nativeObj = outputTexture->getNativeObject();
        if ( ImGui::ImageButton( nativeObj->textureShaderResourceView, ImVec2( 58, 58 ) ) ) {
#elif defined( FLAN_VULKAN ) || defined( FLAN_GL460 )
        if ( ImGui::Button( "balh", ImVec2( 58, 58 ) ) ) {
#endif
            fnString_t filenameBuffer;
            if ( flan::core::DisplayFileOpenPrompt( filenameBuffer, FLAN_STRING( "All (*.dds, *.jpg, *.png, *.png16, *.tga, *.lpng)\0*.dds;*.jpg;*.png;*.png16;*.tga;*.lpng\0DirectDraw Surface (*.dds)\0*.dds\0JPG (*.jpg)\0*.jpg\0PNG (*.png)\0*.png\0PNG 16 Bits (*.png16)\0*.png16\0Low Precision PNG (*.lpng)\0*.lpng\0TGA (*.tga)\0*.tga\0" ), FLAN_STRING( "/." ), FLAN_STRING( "Select a Texture..." ) ) ) {
                fnString_t assetPath;
                flan::core::ExtractFilenameFromPath( filenameBuffer, assetPath );

                outputTexture = g_GraphicsAssetManager->getTexture( ( FLAN_STRING( "GameData/Textures/" ) + assetPath ).c_str() );
            }
        }
    } else {
        if ( ImGui::Button( ( "+##hidden_" + name ).c_str(), ImVec2( 64, 64 ) ) ) {
            fnString_t filenameBuffer;
            if ( flan::core::DisplayFileOpenPrompt( filenameBuffer, FLAN_STRING( "All (*.dds, *.jpg, *.png, *.png16, *.tga, *.lpng)\0*.dds;*.jpg;*.png;*.png16;*.tga;*.lpng\0DirectDraw Surface (*.dds)\0*.dds\0JPG (*.jpg)\0*.jpg\0PNG (*.png)\0*.png\0PNG 16 Bits (*.png16)\0*.png16\0Low Precision PNG (*.lpng)\0*.lpng\0TGA (*.tga)\0*.tga\0" ), FLAN_STRING( "/." ), FLAN_STRING( "Select a Texture..." ) ) ) {
                fnString_t assetPath;
                flan::core::ExtractFilenameFromPath( filenameBuffer, assetPath );

                outputTexture = g_GraphicsAssetManager->getTexture( ( FLAN_STRING( "GameData/Textures/" ) + assetPath ).c_str() );
            }
        }
        texInfosSize.x += 2;
    }

    ImGui::SameLine();

    ImGui::InputTextMultiline( ( "##hidden__TexInfos" + name ).c_str(), &imageInfos.front(), imageInfos.length(), ImVec2( 0, 0 ), ImGuiInputTextFlags_ReadOnly );
}

void flan::framework::DisplayTerrainEditor()
{
    ImGui::RadioButton( "Sculpting", &g_TerrainEditorMode, 0 );
    ImGui::SameLine();
    ImGui::RadioButton( "Paiting", &g_TerrainEditorMode, 1 );
    ImGui::SameLine();
    ImGui::RadioButton( "Foliage Paiting", &g_TerrainEditorMode, 2 );

    if ( g_TerrainEditorMode == 0 ) {
        ImGui::RadioButton( "Raise", &g_TerrainEditorEditionMode, 0 );
        ImGui::SameLine();
        ImGui::RadioButton( "Lower", &g_TerrainEditorEditionMode, 1 );
        ImGui::SameLine();
        ImGui::RadioButton( "Smooth", &g_TerrainEditorEditionMode, 2 );
    }

    if ( ImGui::TreeNode( "Brush Settings" ) ) {
        FLAN_IMGUI_SLIDER_INT( g_TransactionHandler, g_TerrainEditorEditionRadius, 1, 64 )

        if ( g_TerrainEditorMode == 0 ) {
            FLAN_IMGUI_DRAG_FLOAT( g_TransactionHandler, g_TerrainEditorEditionHeight, 0.1f, 0.00001f, 1.0f )
        } else if ( g_TerrainEditorMode == 1 ) {
            DisplayTextureInput( "Base Color (RGB) + Height (A)", g_TerrainEditionBaseMaterialBaseColor );
            DisplayTextureInput( "Normals (tangent space) (RGB) + Roughness (A)", g_TerrainEditionBaseMaterialNormal );

            // TODO User friendly!
            //  Selectable material (links to the biome editor?)
            //  Visual feedback in the panel (albedo texture?)
            FLAN_IMGUI_DRAG_INT( g_TransactionHandler, g_TerrainEditionMaterialIndex1, 1, 0, std::numeric_limits<uint16_t>::max() )
            FLAN_IMGUI_DRAG_INT( g_TransactionHandler, g_TerrainEditionMaterialIndex2, 1, 0, std::numeric_limits<uint16_t>::max() )
        } else if ( g_TerrainEditorMode == 2 ) {
            FLAN_IMGUI_DRAG_FLOAT( g_TransactionHandler, g_TerrainEditorEditionHeight, 0.1f, 0.00001f, 1.0f )
            ImGui::ColorEdit3( "Color", &g_TerrainEditorGrassColor[0] );
        }

        FLAN_IMGUI_DRAG_FLOAT( g_TransactionHandler, g_TerrainEditorEditionHardness, 0.1f, 0.00001f, 1.0f )

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

void flan::framework::UpdateHeightfieldMouseCircle( const Ray& mousePickingRay, Terrain* terrain )
{
    float* const vertices = terrain->getHeightmapValues();

    float pickedHeight = -std::numeric_limits<float>::max();
    int pickedIndex = 0;

    g_RayMarch = mousePickingRay.origin;
    int marchIteration = 0;
    while ( g_RayMarch.y > pickedHeight && marchIteration < 512 ) {
        pickedIndex = ( abs( int( g_RayMarch.x ) ) + abs( int( g_RayMarch.z ) ) * 512 );
        pickedHeight = vertices[pickedIndex];

        g_RayMarch += mousePickingRay.direction;
        marchIteration++;
    }

    dev_TerrainMousePosition = glm::vec3( g_RayMarch.x, pickedHeight, g_RayMarch.z );
}

void flan::framework::EditTerrain( const Ray& mousePickingRay, Terrain* terrain, CommandList* cmdList )
{
    float* const vertices = terrain->getHeightmapValues();

    auto lowX = std::max( 0, abs( int( g_RayMarch.x ) ) - g_TerrainEditorEditionRadius );
    auto lowY = std::max( 0, abs( int( g_RayMarch.z ) ) - g_TerrainEditorEditionRadius );
    auto hiX = std::min( 512, abs( int( g_RayMarch.x ) ) + g_TerrainEditorEditionRadius );
    auto hiY = std::min( 512, abs( int( g_RayMarch.z ) ) + g_TerrainEditorEditionRadius );

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
                        && !IsInRadius( k, int( g_RayMarch.x ), int( g_RayMarch.z ), x, y ) ) {
                        continue;
                    }

                    terrain->setVertexHeight( index, average );
                }
            }
        } else {
            auto editSign = ( ( g_TerrainEditorEditionMode == 0 ) ? +1.0f : -1.0f );
            auto frameHeightUpdate = ( g_TerrainEditorEditionHeight * g_TerrainEditorEditionHardness * editSign );
            
            TerrainEditionHeightDelta += frameHeightUpdate;
            
            for ( int x = lowX; x < hiX; x++ ) {
                for ( int y = lowY; y < hiY; y++ ) {
                    // Attenuation factor
                    float distance = glm::length( glm::vec2( g_RayMarch.x, g_RayMarch.z ) - glm::vec2( x, y ) ) / k;

                    int index = ( x + y * 512 );
                    float& vertexToEdit = vertices[index];

                    if ( !IsInRadius( k, int( g_RayMarch.x ), int( g_RayMarch.z ), x, y ) ) {
                        continue;
                    } else {
                        // For square brush ignore distance attenuation
                        distance = 1.0f;
                    }

                    vertexToEdit += ( frameHeightUpdate * distance );
                    
                    terrain->setVertexHeight( index, vertexToEdit );
                }
            }
        }

        terrain->uploadHeightmap( cmdList );
    } else if ( g_TerrainEditorMode == 2 ) {
        for ( int x = lowX; x < hiX; x++ ) {
            for ( int y = lowY; y < hiY; y++ ) {
                float distance = glm::length( glm::vec2( g_RayMarch.x, g_RayMarch.z ) - glm::vec2( x, y ) ) / k;
                int index = ( x + y * 512 ) * 4;

                if ( g_TerrainEditorBrushType == 1
                  && !IsInRadius( k, int( g_RayMarch.x ), int( g_RayMarch.z ), x, y ) ) {
                    continue;
                }

                terrain->setGrassHeight( index, g_TerrainEditorGrassColor, g_TerrainEditorEditionHardness );
            }
        }

        terrain->uploadGrassmap( cmdList );
    } else {
        for ( int x = lowX; x < hiX; x++ ) {
            for ( int y = lowY; y < hiY; y++ ) {
                float distance = glm::length( glm::vec2( g_RayMarch.x, g_RayMarch.z ) - glm::vec2( x, y ) ) / k;
                int index = ( x + y * 512 ) * 4;

                if ( g_TerrainEditorBrushType == 1
                    && !IsInRadius( k, int( g_RayMarch.x ), int( g_RayMarch.z ), x, y ) ) {
                    continue;
                }

                terrain->setVertexMaterial( index, g_TerrainEditionMaterialIndex1, g_TerrainEditionMaterialIndex2, g_TerrainEditorEditionHardness );
            }
        }

        terrain->uploadSplatmap( cmdList );
    }
}
