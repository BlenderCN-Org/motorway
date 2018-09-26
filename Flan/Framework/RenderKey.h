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

using fnRenderKey_t = uint64_t;

namespace flan
{
    namespace framework
    {
        enum eRenderableType : uint32_t
        {
            RENDERABLE_TYPE_NONE                        = 1 << 0,

            RENDERABLE_TYPE_MESH                        = 1 << 1,
            RENDERABLE_TYPE_DIRECTIONAL_LIGHT           = 1 << 2,
            RENDERABLE_TYPE_POINT_LIGHT                 = 1 << 3,
            RENDERABLE_TYPE_SPHERE_LIGHT                = 1 << 4,
            RENDERABLE_TYPE_SPOT_LIGHT                  = 1 << 5,
            RENDERABLE_TYPE_DISC_LIGHT                  = 1 << 6,
            RENDERABLE_TYPE_RECTANGLE_LIGHT             = 1 << 7,
            RENDERABLE_TYPE_WORLD_VIEWPORT              = 1 << 8,
            RENDERABLE_TYPE_LOCAL_ENVIRONMENT_PROBE     = 1 << 9,
            RENDERABLE_TYPE_GLOBAL_ENVIRONMENT_PROBE    = 1 << 10,
        };

        static fnRenderKey_t BuildRenderKey( const eRenderableType renderType, const fnStringHash_t assetHashcode )
        {
            return ( static_cast<fnRenderKey_t>( assetHashcode ) << 32 ) | renderType;
        }
    }
}
