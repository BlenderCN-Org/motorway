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

#include <Core/StringHelpers.h>

namespace flan
{
    namespace core
    {
        static void ReadString( FileSystemObject* file, fnString_t& string )
        {
            string.clear();

            uint8_t character = '\0';
            file->read( (uint8_t*)&character, sizeof( uint8_t ) );

            while ( file->isGood() && character != '\n' && character != '\r' && character != '\0' && character != 0xFF ) {
                string += character;
                file->read( (uint8_t*)&character, sizeof( uint8_t ) );
            }
        }

        static fnString_t WrappedStringToString( const fnString_t& wrappedString )
        {
            auto stringStart = wrappedString.find_first_of( '"' );
            auto stringEnd = wrappedString.find_first_of( '"', stringStart + 1 );

            if ( stringStart == fnString_t::npos || stringEnd == fnString_t::npos ) {
                return FLAN_STRING( "" );
            }

            auto stringLength = stringEnd - ( stringStart + 1 );

            return wrappedString.substr( ( stringStart + 1 ), stringLength );
        }

        static fnString_t TyplessToString( const fnString_t& typelessString )
        {
            fnString_t lowerCaseString = typelessString;
            flan::core::StringToLower( lowerCaseString );

            auto hashcode = CRC32( lowerCaseString.c_str() );

            if ( hashcode == FLAN_STRING_HASH( "none" ) ) {
                return FLAN_STRING( "" );
            }

            auto unwrappedString = WrappedStringToString( typelessString );

            if ( !unwrappedString.empty() ) {
                return unwrappedString;
            }

            return typelessString;
        }

        static bool StringToBoolean( const fnString_t& string )
        {
            fnString_t lowerCaseString = string;
            StringToLower( lowerCaseString );
            auto hashcode = flan::core::CRC32( lowerCaseString.c_str() );

            return ( hashcode == FLAN_STRING_HASH( "1" ) || hashcode == FLAN_STRING_HASH( "true" ) );
        }

        static glm::vec2 StringTo2DVector( const fnString_t& str )
        {
            if ( str.front() != '{' || str.back() != '}' ) {
                return glm::vec3();
            }

            glm::vec2 vec = {};

            std::size_t offsetX = str.find_first_of( ',' ),
                offsetY = str.find_last_of( ',' ),

                vecEnd = str.find_last_of( '}' );

            fnString_t vecX = str.substr( 1, offsetX - 1 );
            fnString_t vecY = str.substr( offsetX + 1, vecEnd - offsetY - 1 );

            vec.x = std::stof( vecX );
            vec.y = std::stof( vecY );

            return vec;
        }

        static glm::vec3 StringTo3DVector( const fnString_t& str )
        {
            if ( str.size() <= 2 || str.front() != '{' || str.back() != '}' ) {
                return glm::vec3();
            }

            glm::vec3 vec = {};

            std::size_t offsetX = str.find_first_of( ',' ),
                offsetY = str.find_first_of( ',', offsetX + 1 ),
                offsetZ = str.find_last_of( ',' ),

                vecEnd = str.find_last_of( '}' );

            fnString_t vecX = str.substr( 1, offsetX - 1 );
            fnString_t vecY = str.substr( offsetX + 1, offsetY - offsetX - 1 );
            fnString_t vecZ = str.substr( offsetY + 1, vecEnd - offsetZ - 1 );

            vec.x = std::stof( vecX.c_str() );
            vec.y = std::stof( vecY.c_str() );
            vec.z = std::stof( vecZ.c_str() );

            return vec;
        }
    }
}
