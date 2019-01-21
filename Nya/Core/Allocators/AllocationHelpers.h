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

#if NYA_WIN
#include "AllocationHelpersWin32.h"
#endif

namespace nya
{
    namespace core
    {
        inline static void* AlignForward( void* address, const uint8_t alignment )
        {
            return ( void* )( static_cast<uint64_t>( ( ( uint64_t )( address ) + static_cast<uint8_t>( alignment - 1 ) ) )
                              & static_cast<uint8_t>( ~( alignment - 1 ) ) );
        }

        inline static uint8_t AlignForwardAdjustment( const void* address, uint8_t alignment )
        {
            uint8_t adjustment = alignment - ( ( uint64_t )( address )
                                             & static_cast<uint8_t>( alignment - 1 ) );

            if ( adjustment == alignment ) {
                return 0;
            }

            return adjustment;
        }

        static uint8_t AlignForwardAdjustmentWithHeader( const void* address, const uint8_t alignment, const uint8_t headerSize )
        {
            uint8_t adjustment = AlignForwardAdjustment( address, alignment );
            uint8_t neededSpace = headerSize;

            if ( adjustment < neededSpace ) {
                neededSpace -= adjustment;

                // Increase adjustment to fit header 
                adjustment += alignment * ( neededSpace / alignment );

                if ( neededSpace % alignment > 0 ) {
                    adjustment += alignment;
                }
            }

            return adjustment;
        }
    }
}
