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

#include "Ray.h"

template <typename Precision, int ScalarCount>
struct Vector;
using nyaVec3f = Vector<float, 3>;

struct BoundingSphere
{
    nyaVec3f    center;
    float       radius;
};

namespace nya
{
    namespace maths
    {
        void CreateSphere( BoundingSphere& sphere, const nyaVec3f& sphereCenter, const float sphereRadius );
        bool SphereSphereIntersectionTest( const BoundingSphere& left, const BoundingSphere& right );
        bool RaySphereIntersectionTest( const BoundingSphere& sphere, const Ray& ray, float& hitDistance );
    }
}
