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

#include "Vector.h"
#include "Matrix.h"

template<typename Precision>
struct Quaternion
{
    static constexpr int                SCALAR_COUNT = 4;

    static const Quaternion             Zero;
    static const Quaternion             Identity;

    union
    {
        Precision                       scalars[SCALAR_COUNT];

        struct
        {
            union
            {
                Vector<Precision, 3>    axis;
                struct
                {
                    Precision           x;
                    Precision           y;
                    Precision           z;
                };
            };

            Precision                   w;
        };
    };

    // Constructors
    constexpr Quaternion() = default;

    constexpr Quaternion( const Precision x, const Precision y, const Precision z, const Precision w )
        : scalars{ x, y, z, w }
    {

    }

    constexpr Quaternion( const Vector<Precision, 3>& xyz, const Precision w )
        : scalars{ xyz[0], xyz[1], xyz[2], w }
    {

    }

    // Promotions
    template <typename PrecisionR>
    constexpr explicit Quaternion( const Quaternion<PrecisionR>& r )
        : scalars{ ( Precision )r.scalars[0],  ( Precision )r.scalars[1],  ( Precision )r.scalars[2],  ( Precision )r.scalars[3] }
    {

    }

    constexpr Precision& operator[] ( const unsigned int index ) const
    {
        static_assert( index < SCALAR_COUNT );

        return scalars[index];
    }

    constexpr Precision& operator[] ( const unsigned int index )
    {
        static_assert( index < SCALAR_COUNT );

        return scalars[index];
    }

    constexpr Quaternion& operator+ () const
    {
        return *this;
    }

    constexpr Quaternion operator- () const
    {
        return Quaternion( -scalars[0], -scalars[1], -scalars[2], -scalars[3] );
    }

    constexpr Quaternion<Precision>& operator+= ( const Quaternion<Precision>& r )
    {
        scalars[0] += r.scalars[0];
        scalars[1] += r.scalars[1];
        scalars[2] += r.scalars[2];
        scalars[3] += r.scalars[3];

        return *this;
    }

    constexpr Quaternion<Precision>& operator-= ( const Quaternion<Precision>& r )
    {
        scalars[0] -= r.scalars[0];
        scalars[1] -= r.scalars[1];
        scalars[2] -= r.scalars[2];
        scalars[3] -= r.scalars[3];

        return *this;
    }

    constexpr Quaternion<Precision>& operator*= ( const Quaternion<Precision>& r )
    {
        scalars[0] *= r.scalars[0];
        scalars[1] *= r.scalars[1];
        scalars[2] *= r.scalars[2];
        scalars[3] *= r.scalars[3];

        return *this;
    }

    constexpr Quaternion<Precision>& operator*= ( const Precision& r )
    {
        scalars[0] *= r;
        scalars[1] *= r;
        scalars[2] *= r;
        scalars[3] *= r;

        return *this;
    }

    constexpr Quaternion<Precision>& operator/= ( const Precision& r )
    {
        scalars[0] /= r;
        scalars[1] /= r;
        scalars[2] /= r;
        scalars[3] /= r;

        return *this;
    }

    constexpr bool operator== ( const Quaternion<Precision>& r )
    {
        return scalars[0] == r.scalars[0] && scalars[1] == r.scalars[1] && scalars[2] == r.scalars[2] && scalars[3] == r.scalars[3];
    }

    constexpr bool operator!= ( const Quaternion<Precision>& r )
    {
        return scalars[0] != r.scalars[0] || scalars[1] != r.scalars[1] || scalars[2] != r.scalars[2] || scalars[3] != r.scalars[3];
    }

    constexpr Quaternion<Precision> normalize() const
    {
        Quaternion r = *this / length();

        return Quaternion<Precision>( r.scalars[0], r.scalars[1], r.scalars[2], r.scalars[3] );
    }

    constexpr Quaternion<Precision> conjugate() const
    {
        return Quaternion<Precision>( -scalars[0], -scalars[1], -scalars[2], scalars[3] );
    }

    constexpr Quaternion<Precision> inverse() const
    {
        return Quaternion( conjugate() * ( (Precision)1 / lengthSquared() ) );
    }

    constexpr Precision lengthSquared() const
    {
        return scalars[0] * scalars[0] + scalars[1] * scalars[1] + scalars[2] * scalars[2] + scalars[3] * scalars[3];
    }

    constexpr Precision length() const
    {
        return nya::maths::sqrt( lengthSquared() );
    }

    constexpr Matrix<Precision, 4, 4> toMat4x4() const
    {
        Precision twoX( x + x );
        Precision twoY( y + y );
        Precision twoZ( z + z );

        Precision xx2( x * twoX );
        Precision xy2( x * twoY );
        Precision xz2( x * twoZ );
        Precision yy2( y * twoY );
        Precision yz2( y * twoZ );
        Precision zz2( z * twoZ );

        Precision wx2( w * twoX );
        Precision wy2( w * twoY );
        Precision wz2( w * twoZ );

        return Matrix<Precision, 4, 4>(
            (Precision)1 - yy2 - zz2, xy2 + wz2, xz2 - wy2, 0,
            xy2 - wz2, ( Precision )1 - xx2 - zz2, yz2 + wx2, 0,
            xz2 + wy2, yz2 - wx2, ( Precision )1 - xx2 - yy2, 0,
            (Precision)0, ( Precision )0, ( Precision )0, ( Precision )1 );
    }
};

template<typename Precision>
static constexpr Quaternion<Precision> operator / ( const Quaternion<Precision>& l, const Precision& r )
{
    Quaternion<Precision> q = l;
    q /= r;

    return q;
}

template<typename Precision>
static constexpr Quaternion<Precision> operator * ( const Quaternion<Precision>& l, const Precision& r )
{
    Quaternion<Precision> q = l;
    q *= r;

    return q;
}

template<typename Precision>
static constexpr Quaternion<Precision> operator * ( const Precision& l, const Quaternion<Precision>& r )
{
    Quaternion<Precision> q = r;
    q *= l;

    return q;
}

template<typename Precision>
static constexpr Quaternion<Precision> operator * ( const Quaternion<Precision>& l, const Quaternion<Precision>& r )
{
    return Quaternion<Precision>(
        l.scalars[3] * r.scalars[0] + l.scalars[0] * r.scalars[3] + l.scalars[1] * r.scalars[2] - l.scalars[2] * r.scalars[1],
        l.scalars[3] * r.scalars[1] + l.scalars[1] * r.scalars[3] + l.scalars[2] * r.scalars[0] - l.scalars[0] * r.scalars[2],
        l.scalars[3] * r.scalars[2] + l.scalars[2] * r.scalars[3] + l.scalars[0] * r.scalars[1] - l.scalars[1] * r.scalars[0],
        l.scalars[3] * r.scalars[3] - l.scalars[0] * r.scalars[0] - l.scalars[1] * r.scalars[1] - l.scalars[2] * r.scalars[2] );
}

template<typename Precision>
static constexpr Quaternion<Precision> operator + ( const Quaternion<Precision>& l, const Quaternion<Precision>& r )
{
    Quaternion<Precision> q = l;
    q += r;

    return q;
}

template<typename Precision>
static constexpr Quaternion<Precision> operator - ( const Quaternion<Precision>& l, const Quaternion<Precision>& r )
{
    Quaternion<Precision> q = l;
    q -= r;

    return q;
}

#include "Quaternion.inl"

using nyaQuatf = Quaternion<float>;
