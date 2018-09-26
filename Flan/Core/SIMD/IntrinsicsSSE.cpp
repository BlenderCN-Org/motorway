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
#include "Shared.h"
#include "IntrinsicsSSE.h"

#if FLAN_SSE42
fnSIMDVecf_t flan::simd::Create()
{
	return _mm_setzero_ps();
}

fnSIMDVecf_t flan::simd::Load( const float x, const float y, const float z, const float w )
{
    return _mm_set_ps( x, y, z, w );
}

fnSIMDVecf_t flan::simd::LoadUnaligned( const float* floatVec )
{
    return _mm_loadu_ps( floatVec );
}

fnSIMDVecf_t flan::simd::Load( const float xyzw )
{
    return _mm_set1_ps( xyzw );
}

fnSIMDVecf_t flan::simd::Load( const float* floatVec )
{
    return _mm_load1_ps( floatVec );
}

fnSIMDVecf_t flan::simd::Add( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2 )
{
    return _mm_add_ps( v1, v2 );
}

fnSIMDVecf_t flan::simd::Sub( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2 )
{
    return _mm_sub_ps( v1, v2 );
}

fnSIMDVecf_t flan::simd::Mul( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2 )
{
    return _mm_mul_ps( v1, v2 );
}

fnSIMDVecf_t flan::simd::MulAdd( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2, const fnSIMDVecf_t& v3 )
{
    return _mm_add_ps( _mm_mul_ps( v1, v2 ), v3 );
}

fnSIMDVecf_t flan::simd::CompareGreaterMask( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& mask )
{
    return _mm_cmpgt_ps( v1, mask );
}

fnSIMDVecf_t flan::simd::OrMask( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& mask )
{
    return _mm_or_ps( v1, mask );
}
#endif
