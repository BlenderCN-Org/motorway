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

#include <stdint.h>
#include <string>

namespace
{
    template<typename T>
    static uint32_t Core_CRC32Impl( const T* string )
    {
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

namespace flan
{
    namespace core
    {
        static inline uint32_t CRC32( const std::string& string )
        {
            return Core_CRC32Impl( string.c_str() );
        }

        static inline uint32_t CRC32( const std::wstring& string )
        {
            return Core_CRC32Impl( string.c_str() );
        }
    }
}
