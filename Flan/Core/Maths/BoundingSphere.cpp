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
#include "BoundingSphere.h"

void flan::core::CreateSphere( BoundingSphere& sphere, const glm::vec3& sphereCenter, const float sphereRadius )
{
    sphere.center = sphereCenter;
    sphere.radius = sphereRadius;
}

bool flan::core::SphereSphereIntersectionTest( const BoundingSphere& left, const BoundingSphere& right )
{
    float r = left.radius + right.radius;
    r *= r;

    float xDistance = ( left.center.x + right.center.x );
    float yDistance = ( left.center.y + right.center.y );
    float zDistance = ( left.center.z + right.center.z );

    xDistance *= xDistance;
    yDistance *= yDistance;
    zDistance *= zDistance;

    return r < ( xDistance + yDistance + zDistance );
}

bool flan::core::RaySphereIntersectionTest( const BoundingSphere& sphere, const Ray& ray, float& hitDistance )
{
    static constexpr auto VEC3_EPSILON = std::numeric_limits<float>::epsilon();

    auto sphereRadiusSqr = sphere.radius * sphere.radius;

    glm::vec3 diff = sphere.center - ray.origin;
    auto t0 = dot( diff, ray.direction );
    auto dSquared = dot( diff, diff ) - t0 * t0;
    if ( dSquared > sphereRadiusSqr ) {
        return false;
    }

    auto t1 = sqrt( sphereRadiusSqr - dSquared );
    hitDistance = t0 > t1 + VEC3_EPSILON ? t0 - t1 : t0 + t1;
    return hitDistance > VEC3_EPSILON;
}
