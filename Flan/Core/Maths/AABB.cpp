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
#include "AABB.h"

void flan::core::CreateAABBFromMinMaxPoints( AABB& aabb, const glm::vec3& minPoint, const glm::vec3& maxPoint )
{
    aabb.minPoint = minPoint;
    aabb.maxPoint = maxPoint;
}

void flan::core::CreateAABB( AABB& aabb, const glm::vec3& boxCentroid, const glm::vec3& boxHalfExtents )
{
    aabb.minPoint = boxCentroid - boxHalfExtents;
    aabb.maxPoint = boxCentroid + boxHalfExtents;
}

glm::vec3 flan::core::GetAABBHalfExtents( const AABB& aabb )
{
    return ( ( aabb.maxPoint - aabb.minPoint ) * 0.5f );
}

glm::vec3 flan::core::GetAABBCentroid( const AABB& aabb )
{
    auto halfExtents = GetAABBHalfExtents( aabb );

    return ( aabb.minPoint + halfExtents );
}

void flan::core::ExpandAABB( AABB& aabb, const glm::vec3& pointToInclude )
{
    aabb.minPoint = glm::min( aabb.minPoint, pointToInclude );
    aabb.maxPoint = glm::max( aabb.maxPoint, pointToInclude );
}

void flan::core::ExpandAABB( AABB& aabb, const AABB& aabbToInclude )
{
    aabb.minPoint = glm::min( aabb.minPoint, aabbToInclude.minPoint );
    aabb.maxPoint = glm::max( aabb.maxPoint, aabbToInclude.maxPoint );
}

void flan::core::ExpandAABB( AABB& aabb, const BoundingSphere& sphereToInclude )
{
    auto minPoint = sphereToInclude.center - sphereToInclude.radius;
    auto maxPoint = sphereToInclude.center + sphereToInclude.radius;

    aabb.minPoint = glm::min( aabb.minPoint, minPoint );
    aabb.maxPoint = glm::max( aabb.maxPoint, maxPoint );
}

bool flan::core::RayAABBIntersectionTest( const AABB& aabb, const Ray& ray, float& minHit, float& maxHit )
{
    double txMin, txMax, tyMin, tyMax, tzMin, tzMax;

    double invx = 1 / ray.direction.x;
    double tx1 = ( aabb.minPoint.x - ray.origin.x ) * invx;
    double tx2 = ( aabb.maxPoint.x - ray.origin.x ) * invx;
    txMin = glm::min( tx1, tx2 );
    txMax = glm::max( tx1, tx2 );

    double invy = 1 / ray.direction.y;
    double ty1 = ( aabb.minPoint.y - ray.origin.y ) * invy;
    double ty2 = ( aabb.maxPoint.y - ray.origin.y ) * invy;
    tyMin = glm::min( ty1, ty2 );
    tyMax = glm::max( ty1, ty2 );

    double invz = 1 / ray.direction.z;
    double tz1 = ( aabb.minPoint.z - ray.origin.z ) * invz;
    double tz2 = ( aabb.maxPoint.z - ray.origin.z ) * invz;
    tzMin = glm::min( tz1, tz2 );
    tzMax = glm::max( tz1, tz2 );

    double mint = glm::max( txMin, glm::max( tyMin, tzMin ) );
    double maxt = glm::min( txMax, glm::min( tyMax, tzMax ) );

    if ( mint > maxt ) {
        return false;
    }

    minHit = static_cast<float>( mint );
    maxHit = static_cast<float>( maxt );

    return true;
}

uint32_t flan::core::GetMaxDimensionAxisAABB( const AABB& aabb )
{
    auto halfExtents = GetAABBHalfExtents( aabb );

    uint32_t result = 0;

    if ( halfExtents.y > halfExtents.x ) {
        result = 1;

        if ( halfExtents.z > halfExtents.y ) {
            result = 2;
        }
    } else if ( halfExtents.z > halfExtents.x ) {
        result = 2;
    }

    return result;
}
