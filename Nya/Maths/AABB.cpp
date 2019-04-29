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

#include "Helpers.h"
#include "Trigonometry.h"

using namespace nya::maths;

void nya::maths::CreateAABBFromMinMaxPoints( AABB& aabb, const nyaVec3f& minPoint, const nyaVec3f& maxPoint )
{
    aabb.minPoint = minPoint;
    aabb.maxPoint = maxPoint;
}

void nya::maths::CreateAABB( AABB& aabb, const nyaVec3f& boxCentroid, const nyaVec3f& boxHalfExtents )
{
    aabb.minPoint = boxCentroid - boxHalfExtents;
    aabb.maxPoint = boxCentroid + boxHalfExtents;
}

nyaVec3f nya::maths::GetAABBHalfExtents( const AABB& aabb )
{
    return ( ( aabb.maxPoint - aabb.minPoint ) * 0.5f );
}

nyaVec3f nya::maths::GetAABBCentroid( const AABB& aabb )
{
    auto halfExtents = GetAABBHalfExtents( aabb );

    return ( aabb.minPoint + halfExtents );
}

void nya::maths::ExpandAABB( AABB& aabb, const nyaVec3f& pointToInclude )
{
    aabb.minPoint = min( aabb.minPoint, pointToInclude );
    aabb.maxPoint = max( aabb.maxPoint, pointToInclude );
}

void nya::maths::ExpandAABB( AABB& aabb, const AABB& aabbToInclude )
{
    aabb.minPoint = min( aabb.minPoint, aabbToInclude.minPoint );
    aabb.maxPoint = max( aabb.maxPoint, aabbToInclude.maxPoint );
}

void nya::maths::ExpandAABB( AABB& aabb, const BoundingSphere& sphereToInclude )
{
    auto minPoint = sphereToInclude.center - sphereToInclude.radius;
    auto maxPoint = sphereToInclude.center + sphereToInclude.radius;

    aabb.minPoint = min( aabb.minPoint, minPoint );
    aabb.maxPoint = max( aabb.maxPoint, maxPoint );
}

bool nya::maths::RayAABBIntersectionTest( const AABB& aabb, const Ray& ray, float& minHit, float& maxHit )
{
    double txMin, txMax, tyMin, tyMax, tzMin, tzMax;

    double invx = 1 / ray.direction.x;
    double tx1 = ( aabb.minPoint.x - ray.origin.x ) * invx;
    double tx2 = ( aabb.maxPoint.x - ray.origin.x ) * invx;
    txMin = min( tx1, tx2 );
    txMax = max( tx1, tx2 );

    double invy = 1 / ray.direction.y;
    double ty1 = ( aabb.minPoint.y - ray.origin.y ) * invy;
    double ty2 = ( aabb.maxPoint.y - ray.origin.y ) * invy;
    tyMin = min( ty1, ty2 );
    tyMax = max( ty1, ty2 );

    double invz = 1 / ray.direction.z;
    double tz1 = ( aabb.minPoint.z - ray.origin.z ) * invz;
    double tz2 = ( aabb.maxPoint.z - ray.origin.z ) * invz;
    tzMin = min( tz1, tz2 );
    tzMax = max( tz1, tz2 );

    double mint = max( txMin, max( tyMin, tzMin ) );
    double maxt = min( txMax, min( tyMax, tzMax ) );

    if ( mint > maxt ) {
        return false;
    }

    minHit = static_cast<float>( mint );
    maxHit = static_cast<float>( maxt );

    return true;
}

uint32_t nya::maths::GetMaxDimensionAxisAABB( const AABB& aabb )
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
