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
#include "Sampling.h"

float flan::core::RadicalInverseBase2( const uint32_t bits )
{
    uint32_t bitsShifted = ( bits << 16u ) | ( bits >> 16u );
    bitsShifted = ( ( bitsShifted & 0x55555555u ) << 1u ) | ( ( bitsShifted & 0xAAAAAAAAu ) >> 1u );
    bitsShifted = ( ( bitsShifted & 0x33333333u ) << 2u ) | ( ( bitsShifted & 0xCCCCCCCCu ) >> 2u );
    bitsShifted = ( ( bitsShifted & 0x0F0F0F0Fu ) << 4u ) | ( ( bitsShifted & 0xF0F0F0F0u ) >> 4u );
    bitsShifted = ( ( bitsShifted & 0x00FF00FFu ) << 8u ) | ( ( bitsShifted & 0xFF00FF00u ) >> 8u );

    return static_cast< float >( bitsShifted ) * 2.3283064365386963e-10f; // / 0x100000000
}

glm::vec2 flan::core::Hammersley2D( const uint32_t sampleIndex, const uint32_t sampleCount )
{
    return glm::vec2( static_cast< float >( sampleIndex ) / static_cast< float >( sampleCount ), RadicalInverseBase2( sampleIndex ) );
}
