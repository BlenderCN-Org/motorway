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
#include "AABBRasterizer.h"

#include <Core/SIMD/Intrinsics.h>

AABBRasterizer::AABBRasterizer()
{
    viewMatrix = ( __m128* )_aligned_malloc( sizeof( float ) * 4 * 4, 16 );
    projectionMatrix = ( __m128* )_aligned_malloc( sizeof( float ) * 4 * 4, 16 );
}

AABBRasterizer::~AABBRasterizer()
{
    _aligned_free( viewMatrix );
    _aligned_free( projectionMatrix );
}

void AABBRasterizer::setViewProjection( const glm::mat4x4& camViewMatrix, const glm::mat4x4& camProjectionMatrix )
{
    for ( int i = 0; i < 4; i++ ) {
        viewMatrix[i] = flan::simd::LoadUnaligned( ( float* )&camViewMatrix[i] );
    }

    for ( int j = 0; j < 4; j++ ) {
        projectionMatrix[j] = flan::simd::LoadUnaligned( ( float* )&camProjectionMatrix[j] );
    }
}
