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

enum eShaderStage
{
    SHADER_STAGE_VERTEX                     = 1 << 1,
    SHADER_STAGE_PIXEL                      = 1 << 2,
    SHADER_STAGE_TESSELATION_CONTROL        = 1 << 3,
    SHADER_STAGE_TESSELATION_EVALUATION     = 1 << 4,
    SHADER_STAGE_COMPUTE                    = 1 << 5,

    SHADER_STAGE_COUNT                      = 5,

    SHADER_STAGE_ALL                        = ~0
};
