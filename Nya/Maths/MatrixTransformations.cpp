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
#include "Trigonometry.h"

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
        
        nyaMat4x4f MakeRotationMatrix( const float rotation_radians, const nyaVec3f& axis, const nyaMat4x4f& matrix )
        {
            float cos_angle = cos( rotation_radians );
            float sin_angle = sin( rotation_radians );

            nyaVec3f axisNormalized = axis.normalize();
            nyaVec3f tmp( axisNormalized * ( 1.0f - cos_angle ) );

            nyaMat4x4f rotationMatrix( matrix );
            rotationMatrix._00 = cos_angle + tmp[0] * axisNormalized[0];
            rotationMatrix._01 = tmp[0] * axisNormalized[1] + sin_angle * axisNormalized[2];
            rotationMatrix._02 = tmp[0] * axisNormalized[2] - sin_angle * axisNormalized[1];

            rotationMatrix._10 = tmp[1] * axisNormalized[0] - sin_angle * axisNormalized[2];
            rotationMatrix._11 = cos_angle + tmp[1] * axisNormalized[1];
            rotationMatrix._12 = tmp[1] * axisNormalized[2] + sin_angle * axisNormalized[0];

            rotationMatrix._20 = tmp[2] * axisNormalized[0] + sin_angle * axisNormalized[1];
            rotationMatrix._21 = tmp[2] * axisNormalized[1] - sin_angle * axisNormalized[0];
            rotationMatrix._22 = cos_angle + tmp[2] * axisNormalized[2];

            nyaMat4x4f Result;
            Result[0] = matrix[0] * rotationMatrix[0][0] + matrix[1] * rotationMatrix[0][1] + matrix[2] * rotationMatrix[0][2];
            Result[1] = matrix[0] * rotationMatrix[1][0] + matrix[1] * rotationMatrix[1][1] + matrix[2] * rotationMatrix[1][2];
            Result[2] = matrix[0] * rotationMatrix[2][0] + matrix[1] * rotationMatrix[2][1] + matrix[2] * rotationMatrix[2][2];
            Result[3] = matrix[3];
            return Result;
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
        
        nyaMat4x4f MakeOrtho( const float left, const float right, const float bottom, const float top, const float zNear, const float zFar )
        {
            float ReciprocalWidth = 1.0f / ( right - left );
            float ReciprocalHeight = 1.0f / ( top - bottom );
            float fRange = 1.0f / ( zFar - zNear );

            nyaMat4x4f matrix = nyaMat4x4f::Identity;
            matrix._00 = ReciprocalWidth + ReciprocalWidth;
            matrix._11 = ReciprocalHeight + ReciprocalHeight;
            matrix._22 = fRange;

            matrix._30 = -( right + left ) * ReciprocalWidth;
            matrix._31 = -( top + bottom ) * ReciprocalHeight;
            matrix._32 = -fRange * zNear;

            return matrix;
        }
    }
}
