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

// Default Slots [1..7]
// Can be used for single separable pas (e.g. downsampling pass)
constexpr int32_t TEXTURE_SLOT_INDEX_GEOMETRY_DEFAULT               = 1;
constexpr int32_t TEXTURE_SLOT_INDEX_POST_FX_DEFAULT                = 2;
constexpr int32_t TEXTURE_SLOT_INDEX_LINEAR_SAMPLED                 = 3;
constexpr int32_t TEXTURE_SLOT_INDEX_SM_CMP_SAMPLED                 = 4;
constexpr int32_t TEXTURE_SLOT_INDEX_LINEAR_MIRROR_SAMPLED          = 5;
constexpr int32_t TEXTURE_SLOT_INDEX_LINEAR_WRAP_SAMPLED            = 6;
constexpr int32_t TEXTURE_SLOT_INDEX_POINT_SAMPLED                  = 7;

constexpr int32_t TEXTURE_SLOT_INDEX_ACTIVE_ENVMAP = 5;

// Well-known Slots
// Permanent Binded resources
constexpr int32_t TEXTURE_SLOT_INDEX_ACTIVE_ENVMAP_SPECULAR         = 8;
constexpr int32_t TEXTURE_SLOT_INDEX_LIGHT_INDEX_BUFFER             = 9;
constexpr int32_t TEXTURE_SLOT_INDEX_DFG_LUT_DEFAULT                = 10;
constexpr int32_t TEXTURE_SLOT_INDEX_ATMOSPHERE_SCATTERING          = 11;
constexpr int32_t TEXTURE_SLOT_INDEX_ACTIVE_ENVMAP_DIFFUSE          = 12;
constexpr int32_t TEXTURE_SLOT_INDEX_ATMOSPHERE_IRRADIANCE          = 13;
constexpr int32_t TEXTURE_SLOT_INDEX_ATMOSPHERE_TRANSMITTANCE       = 14;
constexpr int32_t TEXTURE_SLOT_INDEX_CSM_TEST                       = 15;

constexpr int32_t TEXTURE_SLOT_INDEX_MATERIAL_BEGIN                 = 17;
