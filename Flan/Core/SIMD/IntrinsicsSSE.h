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

#if FLAN_SSE42
#include <nmmintrin.h>

using fnSIMDVecf_t = __m128;
using fnSIMDVecd_t = __m128d;
using fnSIMDVeci_t = __m128i;

namespace flan 
{
	namespace simd 
	{
        enum eVectorMask : int
        {
            xxxx = 0,
            yyyy = 1,
            zzzz = 2,
            wwww = 3
        };

		fnSIMDVecf_t Create();
		fnSIMDVecf_t Load( const float x, const float y, const float z, const float w );
        fnSIMDVecf_t Load( const float xyzw );
        fnSIMDVecf_t Load( const float* floatVec );

        fnSIMDVecf_t LoadUnaligned( const float* floatVec );

        fnSIMDVecf_t Add( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2 );
        fnSIMDVecf_t Sub( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2 );
        fnSIMDVecf_t Mul( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2 );
        fnSIMDVecf_t MulAdd( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2, const fnSIMDVecf_t& v3 );

        fnSIMDVecf_t CompareGreaterMask( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& mask );
        fnSIMDVecf_t OrMask( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& mask );

        template<eVectorMask mask>
		fnSIMDVecf_t Splat( const fnSIMDVecf_t& v )
		{
			return _mm_shuffle_ps( v, v, _MM_SHUFFLE( mask, mask, mask, mask ) );
		}
	}
}
#endif
