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
#include "Primitives.h"

#include "Helpers.h"
#include "Vector.h"

using namespace nya::maths;

PrimitiveData nya::maths::CreateSpherePrimitive( const uint32_t stacks, const uint32_t slices )
{
    PrimitiveData primitive;

    for ( uint32_t stack = 0; stack <= stacks; stack++ ) {
        float thetaV = PI<float>() * ( ( float )stack / ( float )stacks );
        float r = sin( thetaV );
        float y = cos( thetaV );

        for ( uint32_t slice = 0; slice <= slices; slice++ ) {
            float thetaH = 2.0f * PI<float>() * ( ( float )slice / ( float )slices );
            float x = r * cos( thetaH );
            float z = r * sin( thetaH );

            primitive.vertices.push_back( x );
            primitive.vertices.push_back( y );
            primitive.vertices.push_back( z );

            primitive.vertices.push_back( 0 );
            primitive.vertices.push_back( 1 );
            primitive.vertices.push_back( 0 );

            float u = ( float )slice / ( float )slices;
            float v = ( float )stack / ( float )stacks;

            primitive.vertices.push_back( u );
            primitive.vertices.push_back( 1.0f - v );
        }
    }

    for ( uint32_t stack = 0; stack < stacks; stack++ ) {
        for ( uint32_t slice = 0; slice < slices; slice++ ) {
            int count = slice + ( ( slices + 1 ) * stack );

            primitive.indices.push_back( count );
            primitive.indices.push_back( count + 1 );
            primitive.indices.push_back( count + slices + 2 );

            primitive.indices.push_back( count );
            primitive.indices.push_back( count + slices + 2 );
            primitive.indices.push_back( count + slices + 1 );
        }
    }

    return primitive;
}

PrimitiveData nya::maths::CreateQuadPrimitive()
{
    PrimitiveData primitive;

    primitive.vertices = {
        -1.0f, -1.0f, 1.0f,     +0.0f, +0.0f,
        +1.0f, -1.0f, 1.0f,     +1.0f, +0.0f,
        +1.0f, +1.0f, 1.0f,     +1.0f, +1.0f,
        -1.0f, +1.0f, 1.0f,     +0.0f, +1.0f,
    };

    primitive.indices = {
        0u, 1u, 2u, 2u, 3u, 0u
    };

    return primitive;
}
