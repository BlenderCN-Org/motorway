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
#include "Disk.h"

void flan::core::CreateDisk( Disk& disk, const glm::vec3& diskPosition, const float diskRadius, const glm::vec3& planeNormal )
{
    disk.position = diskPosition;
    disk.normal = planeNormal;
    disk.radius = diskRadius;
    disk.squaredRadius = disk.radius * disk.radius;
}

void flan::core::UpdateDiskRadius( Disk& disk, const float newDiskRadius )
{
    disk.radius = newDiskRadius;
    disk.squaredRadius = disk.radius * disk.radius;
}

bool flan::core::RayDiskIntersectionTest( const Disk& disk, const Ray& ray, float& hitDistance )
{
    hitDistance = 0.0f;

    Plane diskPlane;
    diskPlane.center = disk.position;
    diskPlane.normal = disk.normal;

    if ( flan::core::RayPlaneIntersectionTest( diskPlane, ray, hitDistance ) ) {
        glm::vec3 p = ray.origin + ray.direction * hitDistance;
        glm::vec3 v = p - disk.position;
        float d2 = dot( v, v );

        return ( d2 <= disk.squaredRadius );
    }

    return false;
}
