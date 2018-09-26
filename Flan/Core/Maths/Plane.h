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

struct Plane
{
    glm::vec3   center;
    uint32_t    __PADDING__;
    glm::vec3   normal;
    uint32_t    __PADDING_2__;
};

namespace flan
{
    namespace core
    {
        void CreatePlane( Plane& plane, const glm::vec3& planeCenter, const glm::vec3& planeNormal );
        bool RayPlaneIntersectionTest( const Plane& plane, const Ray& ray, float& hitDistance );
    }
}
