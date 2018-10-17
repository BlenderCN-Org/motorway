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

using fnPageId_t = uint32_t;
inline fnPageId_t MakePageId( const uint32_t x, const uint32_t y, const uint32_t level, const uint32_t texId ) noexcept
{
    return static_cast<fnPageId_t>( ( ( texId & 0xFF ) << 24 ) | ( ( level & 0xFF ) << 16 ) | ( ( y & 0xFF ) << 8 ) | ( x & 0xFF ) );
}

static constexpr fnPageId_t INVALID_PAGEID = 0xFFFFFFFF;
