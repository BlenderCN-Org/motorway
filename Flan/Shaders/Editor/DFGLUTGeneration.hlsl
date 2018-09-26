/*
    Copyright (C) 2018 Team Horsepower
    https://github.com/ProjectHorsepower

    This file is part of Project Horsepower source code.

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
#include "../Shared.hlsli"

RWTexture2D<float4> GeneratedLUT : register( u0 );

static const uint sampleCount = 1024 * 4;

struct Referential
{
    float3 upVector;
    float3 tangentX;
    float3 tangentY;
};

float radicalInverse_VdC( uint bits )
{
    bits = ( bits << 16u ) | ( bits >> 16u );
    bits = ( ( bits & 0x55555555u ) << 1u ) | ( ( bits & 0xAAAAAAAAu ) >> 1u );
    bits = ( ( bits & 0x33333333u ) << 2u ) | ( ( bits & 0xCCCCCCCCu ) >> 2u );
    bits = ( ( bits & 0x0F0F0F0Fu ) << 4u ) | ( ( bits & 0xF0F0F0F0u ) >> 4u );
    bits = ( ( bits & 0x00FF00FFu ) << 8u ) | ( ( bits & 0xFF00FF00u ) >> 8u );
    return float( bits ) * 2.3283064365386963e-10f; // / 0x100000000
}

float2 getSample( uint i, uint N )
{
    return float2( float( i ) / float( N ), radicalInverse_VdC( i ) );
}

Referential CreateReferential( in float3 N )
{
    Referential referential;
    referential.upVector = abs( N.z ) < 0.999 ? float3( 0, 0, 1 ) : float3( 1, 0, 0 );
    referential.tangentX = normalize( cross( referential.upVector, N ) );
    referential.tangentY = cross( N, referential.tangentX );
    return referential;
}

float3 F_Schlick( in float3 f0, in float f90, in float u )
{
    return f0 + ( f90 - f0 ) * pow( 1.f - u, 5.f );
}

float Fr_DisneyDiffuse(
    float NdotV,
    float NdotL,
    float LdotH,
    float linearRoughness )
{
    float energyBias = lerp( 0.0, 0.5, linearRoughness );
    float energyFactor = lerp( 1.0, 1.0 / 1.51, linearRoughness );
    float fd90 = energyBias + 2.0 * LdotH*LdotH * linearRoughness;
    float3 f0 = float3( 1.0f, 1.0f, 1.0f );
    float lightScatter = F_Schlick( f0, fd90, NdotL ).r;
    float viewScatter = F_Schlick( f0, fd90, NdotV ).r;
    return lightScatter * viewScatter * energyFactor;
}

void importanceSampleCosDir(
    in float2 u,
    in float3 N,
    out float3 L,
    out float NdotL,
    out float pdf )
{
    // Local referencial
    float3 upVector = abs( N.z ) < 0.999 ? float3( 0, 0, 1 ) : float3( 1, 0, 0 );
    float3 tangentX = normalize( cross( upVector, N ) );
    float3 tangentY = cross( N, tangentX );
    float u1 = u.x;
    float u2 = u.y;
    float r = sqrt( u1 );
    float phi = u2 * PI * 2;
    L = float3( r*cos( phi ), r*sin( phi ), sqrt( max( 0.0f, 1.0f - u1 ) ) );
    L = normalize( tangentX * L.y + tangentY * L.x + N * L.z );
    NdotL = dot( L, N );
    pdf = NdotL * INV_PI;
}

float3 ImportanceSampleGGX( float2 Xi, float Roughness, float3 N, Referential referential )
{
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt( ( 1 - Xi.y ) / ( 1 + ( a*a - 1 ) * Xi.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );
    float3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;
    // Tangent to world space
    return referential.tangentX * H.x + referential.tangentY * H.y + N * H.z;
}

float G_SmithGGXCorrelated( float NdotL, float NdotV, float alphaG )
{
    float alphaG2 = alphaG * alphaG;
    float Lambda_GGXV = NdotL * sqrt( ( -NdotV * alphaG2 + NdotV ) * NdotV + alphaG2 );
    float Lambda_GGXL = NdotV * sqrt( ( -NdotL * alphaG2 + NdotL ) * NdotL + alphaG2 );
    return 2.0 * NdotL * NdotV / ( Lambda_GGXV + Lambda_GGXL );
}

void importanceSampleGGX_G(
    in float2 u, in float3 V, in float3 N,
    in Referential referential, in float roughness,
    out float NdotH, out float LdotH, out float3 L, out float G )
{
    float3 H = ImportanceSampleGGX( u, roughness, N, referential );
    L = 2 * dot( V, H ) * H - V;
    NdotH = saturate( dot( N, H ) );
    LdotH = saturate( dot( L, H ) );
    float NdotL = saturate( dot( N, L ) );
    float NdotV = saturate( dot( N, V ) );
    G = G_SmithGGXCorrelated( NdotL, NdotV, roughness );
}

float4 IntegrateDFGOnly( in float3 V, in float3 N, in float roughness )
{
    float NdotV = saturate( dot( N, V ) );
    float4 acc = 0;
    float accWeight = 0;

    // Compute pre - integration
    Referential referential = CreateReferential( N );
    for ( uint i = 0; i < sampleCount; ++i ) {
        float2 u = getSample( i, sampleCount );
        float3 L = 0;
        float NdotH = 0;
        float LdotH = 0;
        float G = 0;
        
        // See [ Karis13 ] for implementation
        importanceSampleGGX_G( u, V, N, referential, roughness, NdotH, LdotH, L, G );
        
        // specular GGX DFG preIntegration
        float NdotL = saturate( dot( N, L ) );
        if ( NdotL > 0 && G > 0.0 ) {
            float GVis = G * LdotH / ( NdotH * NdotV );
            float Fc = pow( 1 - LdotH, 5.f );
            acc.x += ( 1 - Fc ) * GVis;
            acc.y += Fc * GVis;
        }
        
        // diffuse Disney preIntegration
        u = frac( u + 0.5 );
        
        float pdf;

        // The pdf is not use because it cancel with other terms
        // ( The 1/ PI from diffuse BRDF and the NdotL from Lambert ’s law ).
        importanceSampleCosDir( u, N, L, NdotL, pdf );

        if ( NdotL >0 ) {
            float LdotH = saturate( dot( L, normalize( V + L ) ) );
            float NdotV = saturate( dot( N, V ) );
            acc.z += Fr_DisneyDiffuse( NdotV, NdotL, LdotH, sqrt( roughness ) );
        }

        accWeight += 1.0;
    }
    
    return acc * ( 1.0f / accWeight );
}

[numthreads( 16, 16, 1 )]
void EntryPointCS( uint2 id : SV_DispatchThreadID )
{
    float roughness = (float)( id.y + 0.5 ) / 512.0;
    float NoV = (float)( id.x + 0.5 ) / 512.0;
    float3 V = float3( sqrt( 1.0f - NoV * NoV ), 0.0f, NoV );
    float3 N = float3( 0.0f, 0.0f, 1.0f );

    GeneratedLUT[int2( id.x, 511 - id.y )] = IntegrateDFGOnly( V, N, roughness );
}
