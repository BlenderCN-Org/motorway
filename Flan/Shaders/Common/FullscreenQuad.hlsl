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
#include <ScreenSpaceShared.hlsli>

ScreenSpaceVertexStageData EntryPointVS( uint id : SV_VERTEXID )
{
    ScreenSpaceVertexStageData output;

    output.TexCoordinates = float2( ( id << 1 ) & 2, id & 2 );
    output.Position = float4( output.TexCoordinates * float2( 2, -2 ) + float2( -1, 1 ), 1, 1 );

    return output;
}
