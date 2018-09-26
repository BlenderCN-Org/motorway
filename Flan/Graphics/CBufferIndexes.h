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

// NOTE Index 0 should not be used (since it can easily be used by mistake)
constexpr int32_t CBUFFER_INDEX_LIGHTBUFFER         = 1;
constexpr int32_t CBUFFER_INDEX_ACTIVE_CAMERA       = 2;
constexpr int32_t CBUFFER_INDEX_MATRICES            = 3;
constexpr int32_t CBUFFER_INDEX_RENDER_INFOS        = 4;
constexpr int32_t CBUFFER_INDEX_ATMOSPHERE          = 5;
constexpr int32_t CBUFFER_INDEX_VOLUMETRIC_CLOUDS   = 6;
constexpr int32_t CBUFFER_INDEX_MATERIAL_EDITOR     = 8;
