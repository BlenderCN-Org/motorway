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
#include "MatrixTransformations.h"

#include "Helpers.h"

namespace nya
{
    namespace maths
    {
        nyaMat4x4f MakeInfReversedZProj( const float fovY_radians, const float aspectWbyH, const float zNear )
        {
            float f = 1.0f / tan( fovY_radians * 0.5f );

            return nyaMat4x4f(
                f / aspectWbyH, 0.0f, 0.0f, 0.0f,
                0.0f, f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, zNear, 0.0f );
        }

        nyaMat4x4f MakeFovProj( const float fovY_radians, const float aspectWbyH, float zNear, float zFar )
        {
            float f = 1.0f / tan( fovY_radians * 0.5f );
            float q = zFar / ( zFar - zNear );

            return nyaMat4x4f(
                f / aspectWbyH, 0.0f, 0.0f, 0.0f,
                0.0f, f, 0.0f, 0.0f,
                0.0f, 0.0f, q, 1.0f,
                0.0f, 0.0f, -q * zNear, 0.0f );
        }

        nyaMat4x4f MakeTranslationMat( const nyaVec3f& translation, const nyaMat4x4f& matrix )
        {
            nyaMat4x4f translationMatrix( matrix );
            translationMatrix[3] = matrix[0] * translation[0] + matrix[1] * translation[1] + matrix[2] * translation[2] + matrix[3];

            return translationMatrix;
        }

        nyaMat4x4f MakeScaleMat( const nyaVec3f& scale, const nyaMat4x4f& matrix )
        {
            nyaMat4x4f scaleMatrix( matrix );
            scaleMatrix[0] = matrix[0] * scale[0];
            scaleMatrix[1] = matrix[1] * scale[1];
            scaleMatrix[2] = matrix[2] * scale[2];
            scaleMatrix[3] = matrix[3];
            
            return scaleMatrix;
        }

        nyaMat4x4f MakeLookAtMat( const nyaVec3f& eye, const nyaVec3f& center, const nyaVec3f& up )
        {
            nyaVec3f zaxis = ( ( center - eye ).normalize() );
            nyaVec3f xaxis = nyaVec3f::cross( up, zaxis ).normalize();
            nyaVec3f yaxis = nyaVec3f::cross( zaxis, xaxis );

            nyaMat4x4f lookAtMat = nyaMat4x4f::Identity;
            lookAtMat[0][0] = xaxis.x;
            lookAtMat[1][0] = xaxis.y;
            lookAtMat[2][0] = xaxis.z;
            lookAtMat[0][1] = yaxis.x;
            lookAtMat[1][1] = yaxis.y;
            lookAtMat[2][1] = yaxis.z;
            lookAtMat[0][2] = zaxis.x;
            lookAtMat[1][2] = zaxis.y;
            lookAtMat[2][2] = zaxis.z;
            lookAtMat[3][0] = -nyaVec3f::dot( xaxis, eye );
            lookAtMat[3][1] = -nyaVec3f::dot( yaxis, eye );
            lookAtMat[3][2] = -nyaVec3f::dot( zaxis, eye );

            return lookAtMat;
        }
    }
}
