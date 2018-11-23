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

RWBuffer<uint> g_DrawInstancedBuffer : register( u0 );

cbuffer BufferCounter : register( b0 )
{
	uint g_NumInstances;
}

[numthreads( 1, 1, 1 )]
void EntryPointCS()
{
	g_DrawInstancedBuffer[0] = 6u * g_NumInstances;// vertexCountPerInstance
	g_DrawInstancedBuffer[1] = 1u; // instanceCount
	g_DrawInstancedBuffer[2] = 0u; // startVertexLocation
	g_DrawInstancedBuffer[3] = 0u; // startInstanceLocation
}
