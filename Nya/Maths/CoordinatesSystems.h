/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

#include "Vector.h"

namespace nya
{
    namespace maths
    {
        static nyaVec3f SphericalToCarthesianCoordinates( const float theta, const float gamma )
        {
            return nyaVec3f( cos( gamma ) * cos( theta ), cos( gamma ) * sin( theta ), sin( gamma ) );
        }

        static nyaVec2f CarthesianToSphericalCoordinates( const nyaVec3f& coords )
        {
            return nyaVec2f( atan2( coords.x, coords.y ), atan2( hypot( coords.x, coords.y ), coords.z ) );
        }
    }
}