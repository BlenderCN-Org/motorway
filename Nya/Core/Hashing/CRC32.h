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

#include <stdint.h>
#include <string>
#include <nmmintrin.h>

namespace
{
    template<typename T>
    static uint32_t Core_CRC32Impl( const T* string, const std::size_t length )
    {
       /* unsigned int hashcode = ~0;

        for ( std::size_t i = 0; i < length; i += 4 ) {
            unsigned int final = 0;
            final |= ( string[i + 0] << 24 );
            final |= ( string[i + 1] << 16 );
            final |= ( string[i + 2] << 8 );
            final |= ( string[i + 3] );

            hashcode = _mm_crc32_u32( hashcode, final );
        }

        return hashcode;*/

        unsigned int hashcode = ~0;

        for ( const T* character = string; *character; ++character ) {
            hashcode ^= *character;

            hashcode = hashcode & 1 ? ( hashcode >> 1 ) ^ POLYNOMIAL : hashcode >> 1;
            hashcode = hashcode & 1 ? ( hashcode >> 1 ) ^ POLYNOMIAL : hashcode >> 1;
            hashcode = hashcode & 1 ? ( hashcode >> 1 ) ^ POLYNOMIAL : hashcode >> 1;
            hashcode = hashcode & 1 ? ( hashcode >> 1 ) ^ POLYNOMIAL : hashcode >> 1;
            hashcode = hashcode & 1 ? ( hashcode >> 1 ) ^ POLYNOMIAL : hashcode >> 1;
            hashcode = hashcode & 1 ? ( hashcode >> 1 ) ^ POLYNOMIAL : hashcode >> 1;
            hashcode = hashcode & 1 ? ( hashcode >> 1 ) ^ POLYNOMIAL : hashcode >> 1;
            hashcode = hashcode & 1 ? ( hashcode >> 1 ) ^ POLYNOMIAL : hashcode >> 1;
        }

        return ~hashcode;
    }
}

namespace nya
{
    namespace core
    {
        static inline uint32_t CRC32( const std::string& string )
        {
            return Core_CRC32Impl( string.c_str(), string.size() );
        }

        static inline uint32_t CRC32( const std::wstring& string )
        {
            return Core_CRC32Impl( string.c_str(), string.size() );
        }
    }
}
