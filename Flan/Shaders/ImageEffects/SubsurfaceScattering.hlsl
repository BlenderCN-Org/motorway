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
#include "SeparableSSS.h"
#include <Colormetry.hlsli>

struct psDataScreenQuad_t
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

Texture2D g_TexColor : register( t0 );
Texture2D g_TexDepth : register( t1 );
Texture2D g_TexGBuffer : register( t2 );

float4 EntryPointPS( psDataScreenQuad_t p ) : SV_TARGET
{
#if PH_FIRST_PASS
    static const float2 DIRECTION = float2( 1.0f, 0.0f );
    static const bool IS_FIRST_PASS = true;
#else
    static const float2 DIRECTION = float2( 0.0f, 1.0f );
    static const bool IS_FIRST_PASS = false;
#endif
    
    return SSSSBlurPS( p.uv, g_TexColor, g_TexDepth, g_TexGBuffer, 0.005f, DIRECTION, IS_FIRST_PASS );
}
