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

namespace
{
    static constexpr int SQRT_ITERATION_COUNT = 128;

    template<typename T>
    constexpr T sqrt_eps() 
    {
        return ( T )0.00001;
    }

    template<typename T>
    constexpr T r_sqrt( const T x, const T x_half, const int itCount )
    {
        return ( nya::maths::abs( x_half - x / x_half ) / ( ( T )1 + x_half ) < sqrt_eps<T>() ) ? 
            ( itCount < SQRT_ITERATION_COUNT ) ? r_sqrt( x, ( T )0.5f * ( x_half + x / x_half ), ( itCount + 1 ) ) : x_half : x_half;
    }
}

namespace nya
{
    namespace maths
    {
        template< typename T > constexpr T abs( const T x );

        template< typename T >
        constexpr T sqrt( const T x )
        {
            return ( x < ( T )0 ) ? ( T )-1 : sqrt_eps<T>() > abs( x ) ? ( T )0 : sqrt_eps<T>() > abs( ( T )1 - x ) ? x : r_sqrt( x, x / ( T )2, 0 );
        }
    }
}