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
#include <Shared.h>
#include "XorShift.h"

static uint64_t g_XorShiftSeed = 0xB00B1E51B00B5;

uint64_t flan::core::XorShiftNext()
{
    // xorshift*
    g_XorShiftSeed ^= ( g_XorShiftSeed >> 12 );
    g_XorShiftSeed ^= ( g_XorShiftSeed << 25 );
    g_XorShiftSeed ^= ( g_XorShiftSeed >> 27 );

    return ( g_XorShiftSeed * 0x2545F4914F6CDD1Dll );
}

float flan::core::XorShiftNext01()
{
    return static_cast< float >( static_cast< double >( XorShiftNext() ) / std::numeric_limits<uint64_t>::max() );
}
