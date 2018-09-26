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
#include "BoundingSphere.h"

struct AABB
{
    glm::vec3 minPoint;
    uint32_t  __PADDING1__;
    glm::vec3 maxPoint;
    uint32_t  __PADDING2__;
};

namespace flan
{
    namespace core
    {
        void CreateAABB( AABB& aabb, const glm::vec3& boxCentroid, const glm::vec3& boxHalfExtents );
        void CreateAABBFromMinMaxPoints( AABB& aabb, const glm::vec3& minPoint, const glm::vec3& maxPoint );

        glm::vec3 GetAABBHalfExtents( const AABB& aabb );
        glm::vec3 GetAABBCentroid( const AABB& aabb );

        void ExpandAABB( AABB& aabb, const glm::vec3& pointToInclude );
        void ExpandAABB( AABB& aabb, const AABB& aabbToInclude );
        void ExpandAABB( AABB& aabb, const BoundingSphere& sphereToInclude );

        bool RayAABBIntersectionTest( const AABB& aabb, const Ray& ray, float& minHit, float& maxHit );

        uint32_t GetMaxDimensionAxisAABB( const AABB& aabb );
    }
}
