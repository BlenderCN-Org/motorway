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

#include "Matrix.h"

namespace nya
{
    namespace maths
    {
        nyaMat4x4f  MakeInfReversedZProj( const float fovY_radians, const float aspectWbyH, const float zNear );
        nyaMat4x4f  MakeFovProj( const float fovY_radians, const float aspectWbyH, float zNear, float zFar );

        nyaMat4x4f  MakeTranslationMat( const nyaVec3f& translation, const nyaMat4x4f& matrix = nyaMat4x4f::Identity );
        nyaMat4x4f  MakeScaleMat( const nyaVec3f& scale, const nyaMat4x4f& matrix = nyaMat4x4f::Identity );
    
        nyaMat4x4f  MakeLookAtMat( const nyaVec3f& eye, const nyaVec3f& center, const nyaVec3f& up );
        nyaMat4x4f  MakeOrtho( const float left, const float right, const float bottom, const float top, const float zNear, const float zFar );
    }
}
