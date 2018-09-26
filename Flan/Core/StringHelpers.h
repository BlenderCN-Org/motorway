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

namespace flan
{
    namespace core
    {
        static void TrimString( fnString_t& str )
        {
            size_t charPos = str.find_first_not_of( FLAN_STRING( "     \n" ) );

            if ( charPos != fnString_t::npos ) {
                str.erase( 0, charPos );
            }

            charPos = str.find_last_not_of( FLAN_STRING( "     \n" ) );

            if ( charPos != fnString_t::npos ) {
                str.erase( charPos + 1 );
            }

            // Remove carriage return
            str.erase( std::remove( str.begin(), str.end(), '\r' ), str.end() );
            str.erase( std::remove( str.begin(), str.end(), '\t' ), str.end() );
        }
        
        static void RemoveWordFromString( fnString_t& string, const fnString_t& word, const fnString_t& newWord = FLAN_STRING( "" ) )
        {
            std::size_t wordIndex = 0;

            while ( ( wordIndex = string.find( word ) ) != fnString_t::npos ) {
                string.replace( wordIndex, word.size(), newWord );
            }
        }

        inline void StringToLower( fnString_t& string )
        {
            std::transform( string.begin(), string.end(), string.begin(), ::tolower );
        }

        inline void StringToUpper( fnString_t& string )
        {
            std::transform( string.begin(), string.end(), string.begin(), ::toupper );
        }

        static fnString_t GetFileExtensionFromPath( const fnString_t& path )
        {
            const size_t dotPosition = path.find_last_of( '.' );

            if ( dotPosition != fnString_t::npos ) {
                return path.substr( dotPosition + 1, path.length() - dotPosition );
            }

            return FLAN_STRING( "" );
        }
    }
}
