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

struct Instance
{
	float3	position;
	float 	specular;
	float3	albedo;
	uint	vertexOffsetAndSkew;
	float2	rotation;
	float2	scale;
};

cbuffer GrassGenerationBuffer : register( b0 )
{
	float4 	 g_CameraFrustumPlanes[6]; // NOTE This is main camera frustum (NOT top down camera!)
	float3	 g_CameraPositionWorldSpace;
}

Texture2D<float4> 	g_TexGrassMap  		: register( t0 ); // RGB color A height
Texture2D<float4> 	g_TexTopDownCapture : register( t1 );

[numthreads( 32, 32, 1 )]
void EntryPointCS( uint3 DispatchThreadID : SV_DISPATCHTHREADID )
{
	// Cull 32 texel per thread (assuming 2k x 2k grass map)
	float4 grassMapSample = g_TexGrassMap.Load( uint3( DispatchThreadID.xy, 0 ) );
	
	
}
