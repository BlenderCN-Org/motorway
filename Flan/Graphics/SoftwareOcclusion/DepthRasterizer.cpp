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
#include "DepthRasterizer.h"

static constexpr int MAX_TRIS_IN_BIN_ST = 1024 * 16;
static constexpr int NUM_XFORMVERTS_TASKS = 16;

DepthRasterizer::DepthRasterizer()
    : fbWidth( 0 )
    , fbHeight( 0 )
    , fbTileWidth( 0 )
    , fbTileHeight( 0 )
    , fbTileCountWidth( 0 )
    , fbTileCountHeight( 0 )
    , fbTileCount( 0 )
    , yOffset1( 0 )
    , xOffset1( 0 )
    , yOffset2( 0 )
    , xOffset2( 0 )
    , viewportMatrix( 1.0f )
    , depthBuffer{ nullptr, nullptr }
{

}

DepthRasterizer::~DepthRasterizer()
{

}

void DepthRasterizer::setFramebufferSize( const uint32_t width, const uint32_t height )
{
    // Calculate framebuffer infos
    fbWidth = width;
    fbHeight = height;

    fbTileWidth = static_cast<uint32_t>( fbWidth / 4.0f );
    fbTileHeight = static_cast<uint32_t>( fbHeight / 8.0f );

    fbTileCountWidth = ( fbWidth + fbTileWidth - 1 ) / fbTileWidth;
    fbTileCountHeight = ( fbHeight + fbTileHeight - 1 ) / fbTileHeight;

    fbTileCount = fbTileCountWidth * fbTileCountHeight;

    yOffset1 = fbTileCountWidth;
    xOffset1 = 1;

    yOffset2 = fbTileCountWidth * MAX_TRIS_IN_BIN_ST;
    xOffset2 = MAX_TRIS_IN_BIN_ST;

    // Prepare bins for rasterized geometry
    for ( int i = 0; i < BUFFER_COUNT; i++ ) {
        indicesBins[i] = new uint32_t[fbTileCount * MAX_TRIS_IN_BIN_ST];
        meshIndices[i] = new USHORT[fbTileCount * MAX_TRIS_IN_BIN_ST];
        triangleCountInBins[i] = new USHORT[fbTileCount];
    }

    // View Matrix
    viewportMatrix = glm::mat4x4( 
        0.5f * ( float )fbWidth, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f * ( float )fbHeight, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f * ( float )fbWidth, 0.5f * ( float )fbHeight, 0.0f, 1.0f );

    // Allocate depth buffer texels (double buffered)
    for ( int i = 0; i < BUFFER_COUNT; i++ ) {
        if ( depthBuffer[i] != nullptr ) {
            _aligned_free( depthBuffer[i] );
        }

        depthBuffer[i] = static_cast< int8_t* >( _aligned_malloc( sizeof( int8_t ) * fbWidth * fbHeight * 4, 16 ) );
        memset( depthBuffer[i], 0, sizeof( int8_t ) * fbWidth * fbHeight * 4 );
    }
}
