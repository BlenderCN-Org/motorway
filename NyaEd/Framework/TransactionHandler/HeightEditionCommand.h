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

#include "TransactionCommand.h"

//#include <Framework/Terrain.h>
//
//#include <algorithm>
//
//static inline bool isInRadius( const float rSquared, const int x, const int y, const int pointX, const int pointY )
//{
//    auto xSquared = ( pointX - x );
//    xSquared *= xSquared;
//
//    auto ySquared = ( pointY - y );
//    ySquared *= ySquared;
//
//    return ( xSquared + ySquared ) < rSquared;
//}
//
//class HeightEditionCommand : public TransactionCommand
//{
//public:
//    // NOTE editionHeightWithSign should be height * hardness * sign (-1 to lower; +1 to raise)
//    HeightEditionCommand( Terrain* terrain, const glm::vec3& mouseToTerrainCoords, const int editionRadius, const float editionHeightWithSign )
//        : editedTerrain( terrain )
//        , mouseCoordinates( mouseToTerrainCoords )
//        , radius( editionRadius )
//        , editionHeight( editionHeightWithSign )
//        , isFirstEdit( true )
//    {
//        lowX = std::max( 0, abs( int( mouseToTerrainCoords.x ) ) - editionRadius );
//        lowY = std::max( 0, abs( int( mouseToTerrainCoords.z ) ) - editionRadius );
//        maxX = std::min( 512, abs( int( mouseToTerrainCoords.x ) ) + editionRadius );
//        maxY = std::min( 512, abs( int( mouseToTerrainCoords.z ) ) + editionRadius );
//
//        actionInfos = "Edit Heightfield Height";
//    }
//
//    virtual void execute() override
//    {
//        if ( isFirstEdit ) {
//            isFirstEdit = false;
//            return;
//        }
//
//        updateHeight( +1.0f );
//    }
//
//    virtual void undo() override
//    {
//        updateHeight( -1.0f );
//    }
//
//private:
//    // Edition Bounds (world units)
//    int         lowX;
//    int         lowY;
//    int         maxX;
//    int         maxY;
//
//    const glm::vec3     mouseCoordinates;
//    Terrain*            editedTerrain;
//    const int           radius;
//    float               editionHeight;
//
//    bool isFirstEdit;
//
//private:
//    void updateHeight( const float actionSign )
//    {
//        float* const vertices = editedTerrain->getHeightmapValues();
//
//        const float k = static_cast< float >( radius * radius );
//
//        for ( int x = lowX; x < maxX; x++ ) {
//            for ( int y = lowY; y < maxY; y++ ) {
//                // Attenuation factor
//                float distance = glm::length( glm::vec2( mouseCoordinates.x, mouseCoordinates.z ) - glm::vec2( x, y ) ) / k;
//
//                int index = ( x + y * 512 );
//                float& vertexToEdit = vertices[index];
//
//                if ( !isInRadius( k, int( mouseCoordinates.x ), int( mouseCoordinates.z ), x, y ) ) {
//                    continue;
//                }
//
//                vertexToEdit += ( editionHeight * distance * actionSign );
//                editedTerrain->setVertexHeight( index, vertexToEdit );
//            }
//        }
//    }
//};
