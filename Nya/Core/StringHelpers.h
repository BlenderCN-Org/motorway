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

#include <string>
#include <algorithm>

#include "Types.h"

namespace nya
{
    namespace core
    {
        static void TrimString( nyaString_t& str )
        {
            size_t charPos = str.find_first_not_of( NYA_STRING( "     \n" ) );

            if ( charPos != nyaString_t::npos ) {
                str.erase( 0, charPos );
            }

            charPos = str.find_last_not_of( NYA_STRING( "     \n" ) );

            if ( charPos != nyaString_t::npos ) {
                str.erase( charPos + 1 );
            }

            // Remove carriage return
            str.erase( std::remove( str.begin(), str.end(), '\r' ), str.end() );
            str.erase( std::remove( str.begin(), str.end(), '\t' ), str.end() );
        }

        static void RemoveWordFromString( nyaString_t& string, const nyaString_t& word, const nyaString_t& newWord = NYA_STRING( "" ) )
        {
            std::size_t wordIndex = 0;

            while ( ( wordIndex = string.find( word ) ) != nyaString_t::npos ) {
                string.replace( wordIndex, word.size(), newWord );
            }
        }

        inline void StringToLower( nyaString_t& string )
        {
            std::transform( string.begin(), string.end(), string.begin(), ::tolower );
        }

        inline void StringToUpper( nyaString_t& string )
        {
            std::transform( string.begin(), string.end(), string.begin(), ::toupper );
        }

        static nyaString_t GetFileExtensionFromPath( const nyaString_t& path )
        {
            const size_t dotPosition = path.find_last_of( '.' );

            if ( dotPosition != nyaString_t::npos ) {
                return path.substr( dotPosition + 1, path.length() - dotPosition );
            }

            return NYA_STRING( "" );
        }

#if NYA_UNICODE
        inline std::string WideStringToString( const nyaString_t& str )
        {
            return std::string( str.begin(), str.end() );
        }
#else
        inline std::string WideStringToString( const nyaString_t& str )
        {
            return str;
        }
#endif
    }
}
