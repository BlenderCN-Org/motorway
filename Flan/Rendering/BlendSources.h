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

#include <Core/LazyEnum.h>

namespace flan
{
    namespace rendering
    {
#define BlendSource( option )\
        option( BLEND_SOURCE_ZERO )\
        option( BLEND_SOURCE_ONE )\
        option( BLEND_SOURCE_SRC_COLOR )\
        option( BLEND_SOURCE_INV_SRC_COLOR )\
        option( BLEND_SOURCE_SRC_ALPHA )\
        option( BLEND_SOURCE_INV_SRC_ALPHA )\
        option( BLEND_SOURCE_DEST_ALPHA )\
        option( BLEND_SOURCE_INV_DEST_ALPHA )\
        option( BLEND_SOURCE_DEST_COLOR )\
        option( BLEND_SOURCE_INV_DEST_COLOR )\
        option( BLEND_SOURCE_SRC_ALPHA_SAT )\
        option( BLEND_SOURCE_BLEND_FACTOR )\
        option( BLEND_SOURCE_INV_BLEND_FACTOR )

        FLAN_LAZY_ENUM( BlendSource )
#undef BlendSource
    }
}
