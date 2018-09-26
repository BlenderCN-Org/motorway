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

#include <glm/glm.hpp>

namespace flan
{
    namespace core
    {
        inline static glm::vec3 SphericalToCarthesianCoordinates( const float theta, const float gamma )
        {
            return glm::vec3( cos( gamma ) * cos( theta ), cos( gamma ) * sin( theta ), sin( gamma ) );
        } 
        
        inline static glm::vec2 CarthesianToSphericalCoordinates( const glm::vec3 coords )
        {
            return glm::vec2( atan2( coords.x, coords.y ), atan2( hypot( coords.x, coords.y ), coords.z ) );
        }
    }
}
