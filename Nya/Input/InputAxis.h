/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

namespace nya
{
    namespace input
    {
#define InputAxis( option )\
        option( AXIS_NONE )\
        option( MOUSE_X )\
        option( MOUSE_Y )\
        option( MOUSE_SCROLL_WHEEL )\
        option( JOYSTICK_X )\
        option( JOYSTICK_X1 )\
        option( JOYSTICK_Y )\
        option( JOYSTICK_Y1 )\
        option( JOYSTICK_Z )\
        option( JOYSTICK_Z1 )\

        NYA_LAZY_ENUM( InputAxis )
#undef InputAxis
    }
}
