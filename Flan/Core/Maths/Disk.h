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
#include "Ray.h"
#include "Plane.h"

struct Disk
{
    glm::vec3 position;
    float radius;
    glm::vec3 normal;
    float squaredRadius;
};

namespace flan
{
    namespace core
    {
        void CreateDisk( Disk& disk, const glm::vec3& diskPosition, const float diskRadius, const glm::vec3& planeNormal );
        void UpdateDiskRadius( Disk& disk, const float newDiskRadius );
        bool RayDiskIntersectionTest( const Disk& disk, const Ray& ray, float& hitDistance );
    }
}
