#include <Shared.h>
#include "IntrinsicsSSE.h"

#if NYA_SSE42
fnSIMDVecf_t nya::simd::Create()
{
    return _mm_setzero_ps();
}

fnSIMDVecf_t nya::simd::Load( const float x, const float y, const float z, const float w )
{
    return _mm_set_ps( x, y, z, w );
}

fnSIMDVecf_t nya::simd::LoadUnaligned( const float* floatVec )
{
    return _mm_loadu_ps( floatVec );
}

fnSIMDVecf_t nya::simd::Load( const float xyzw )
{
    return _mm_set1_ps( xyzw );
}

fnSIMDVecf_t nya::simd::Load( const float* floatVec )
{
    return _mm_load1_ps( floatVec );
}

fnSIMDVecf_t nya::simd::Add( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2 )
{
    return _mm_add_ps( v1, v2 );
}

fnSIMDVecf_t nya::simd::Sub( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2 )
{
    return _mm_sub_ps( v1, v2 );
}

fnSIMDVecf_t nya::simd::Mul( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2 )
{
    return _mm_mul_ps( v1, v2 );
}

fnSIMDVecf_t nya::simd::MulAdd( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& v2, const fnSIMDVecf_t& v3 )
{
    return _mm_add_ps( _mm_mul_ps( v1, v2 ), v3 );
}

fnSIMDVecf_t nya::simd::CompareGreaterMask( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& mask )
{
    return _mm_cmpgt_ps( v1, mask );
}

fnSIMDVecf_t nya::simd::OrMask( const fnSIMDVecf_t& v1, const fnSIMDVecf_t& mask )
{
    return _mm_or_ps( v1, mask );
}
#endif
