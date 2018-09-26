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
    namespace graphics
    {
#define RenderList( list )\
        list( RENDER_LIST_DEFAULT )\
        list( RENDER_LIST_OPAQUE )\
        list( RENDER_LIST_TRANSPARENT )\
        list( RENDER_LIST_UI )

        FLAN_LAZY_ENUM( RenderList )
#undef RenderList
    }
}
