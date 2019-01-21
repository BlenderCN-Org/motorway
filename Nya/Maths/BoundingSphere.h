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

#include <glm/glm/glm.hpp>
#include "Ray.h"

struct BoundingSphere
{
    glm::vec3   center;
    float       radius;
};

namespace nya
{
    namespace core
    {
        void CreateSphere( BoundingSphere& sphere, const glm::vec3& sphereCenter, const float sphereRadius );
        bool SphereSphereIntersectionTest( const BoundingSphere& left, const BoundingSphere& right );
        bool RaySphereIntersectionTest( const BoundingSphere& sphere, const Ray& ray, float& hitDistance );
    }
}
