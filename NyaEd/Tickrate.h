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

namespace nya
{
    namespace editor
    {
        // As ticks
        constexpr uint32_t LOGIC_TICKRATE   = 100;
        constexpr uint32_t PHYSICS_TICKRATE = 100;

        // As milliseconds
        constexpr float LOGIC_DELTA = 1.0f / static_cast<float>( LOGIC_TICKRATE );
        constexpr float PHYSICS_DELTA = 1.0f / static_cast<float>( PHYSICS_TICKRATE );
    }
}
