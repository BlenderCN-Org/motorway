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
#include <Shared.hlsli>
#include <Colormetry.hlsli>

SamplerState LinearSampler { Filter = MIN_MAG_LINEAR_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };
Texture2D g_TexMainRenderTarget : register( t0 );

float4 EntryPointPS( psDataScreenQuad_t p ) : SV_TARGET
{
    float3 color = g_TexMainRenderTarget.Sample( LinearSampler, p.uv ).rgb;
    float luminance = rgbToLuminance( color.rgb );
    float intensity = 0.10f * max( luminance - 1.0f, 0.0f );
    
    return float4( color * intensity, 1.0f );
}
