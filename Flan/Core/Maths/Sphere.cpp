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
#include "Sphere.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

void flan::core::CreateSphere( Sphere& sphere, const uint32_t stacks, const uint32_t slices )
{
    float radius = 1.0f;
    bool flipUVHorizontal = false;

    for ( uint32_t stack = 0; stack <= stacks; stack++ ) {
        float thetaV = glm::pi<float>() * ( ( float )stack / ( float )stacks );
        float r = radius * sin( thetaV );
        float y = radius * cos( thetaV );
        for ( uint32_t slice = 0; slice <= slices; slice++ ) {
            float thetaH = 2.0f * glm::pi<float>() * ( ( float )slice / ( float )slices );
            float x = r * cos( thetaH );
            float z = r * sin( thetaH );
            sphere.vertices.push_back( x );
            sphere.vertices.push_back( y );
            sphere.vertices.push_back( z );

            float u = ( float )slice / ( float )slices;
            if ( flipUVHorizontal ) {
                u = 1.0f - u;
            }
            float v = ( float )stack / ( float )stacks;
            sphere.vertices.push_back( u );
            sphere.vertices.push_back( 1.0f - v );
        }
    }

    for ( uint32_t stack = 0; stack < stacks; stack++ ) {
        for ( uint32_t slice = 0; slice < slices; slice++ ) {
            int count = slice + ( ( slices + 1 ) * stack );

            sphere.indices.push_back( count );
            sphere.indices.push_back( count + 1 );
            sphere.indices.push_back( count + slices + 2 );

            sphere.indices.push_back( count );
            sphere.indices.push_back( count + slices + 2 );
            sphere.indices.push_back( count + slices + 1 );
        }
    }
}
