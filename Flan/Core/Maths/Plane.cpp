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
#include "Plane.h"

void flan::core::CreatePlane( Plane& plane, const glm::vec3& planeCenter, const glm::vec3& planeNormal )
{
    plane.center = planeCenter;
    plane.normal = planeNormal;
}

bool flan::core::RayPlaneIntersectionTest( const Plane& plane, const Ray& ray, float& hitDistance )
{
    // assuming vectors are all normalized
    float denom = glm::dot( plane.normal, ray.direction );
    if ( denom > std::numeric_limits<float>::epsilon() ) {
        glm::vec3 p0l0 = plane.center - ray.origin;
        hitDistance = glm::dot( p0l0, plane.normal ) / denom;

        return ( hitDistance >= 0.0f );
    }

    return false;
}
