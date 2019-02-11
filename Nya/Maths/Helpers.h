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
    static constexpr double _PI = 3.14159265358979323846;
}

namespace nya
{
    namespace maths
    {
        template< typename T >
        constexpr T PI()
        {
            return (T)_PI;
        }

        template< typename T >
        constexpr T TWO_PI()
        {
            return ( T )2 * PI<T>();
        }

        template< typename T >
        constexpr T HALF_PI()
        {
            return PI<T>() / (T)2;
        }

        template< typename T >
        inline constexpr T radians( const T x )
        {
            return ( x * PI<T>() ) / ( T )180;
        }

        template< typename T >
        inline constexpr T degrees( const T x )
        {
            return ( x * ( T )180 ) / PI<T>();
        }

        template< typename T, typename R >
        inline constexpr T min( const T a, const R b )
        {
            return static_cast< T >( ( a < b ) ? a : b );
        }

        template< typename T, typename R >
        inline constexpr T max( const T a, const R b )
        {
            return static_cast< T >( ( a > b ) ? a : b );
        } 
        
        template<typename T>
        constexpr T clamp( const T x, const T minVal, const T maxVal)
        {
            return min( max( x, minVal ), maxVal );
        }

        template< typename T >
        constexpr T abs( const T x )
        {
            return ( x == ( T )0 ? ( T )0 : ( ( x < ( T )0 ) ? -x : x ) );
        }

        template<typename T>
        constexpr T lerp( const T x, const T y, const float a )
        {
            return x * ( 1.0f - a ) + y * a;
        }
    }
}