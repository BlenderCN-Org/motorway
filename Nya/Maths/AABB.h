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
#include "BoundingSphere.h"

struct AABB
{
    nyaVec3f minPoint;
    uint32_t  __PADDING1__;
    nyaVec3f maxPoint;
    uint32_t  __PADDING2__;
};

namespace nya
{
    namespace maths
    {
        void CreateAABB( AABB& aabb, const nyaVec3f& boxCentroid, const nyaVec3f& boxHalfExtents );
        void CreateAABBFromMinMaxPoints( AABB& aabb, const nyaVec3f& minPoint, const nyaVec3f& maxPoint );

        nyaVec3f GetAABBHalfExtents( const AABB& aabb );
        nyaVec3f GetAABBCentroid( const AABB& aabb );

        void ExpandAABB( AABB& aabb, const nyaVec3f& pointToInclude );
        void ExpandAABB( AABB& aabb, const AABB& aabbToInclude );
        void ExpandAABB( AABB& aabb, const BoundingSphere& sphereToInclude );

        bool RayAABBIntersectionTest( const AABB& aabb, const Ray& ray, float& minHit, float& maxHit );

        uint32_t GetMaxDimensionAxisAABB( const AABB& aabb );
    }
}
