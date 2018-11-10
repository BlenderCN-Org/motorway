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
#include "Shared.h"
#include "String.h"

#include <Core/Allocators/BaseAllocator.h>

namespace flan
{
    string::string( BaseAllocator* allocator )
        : stringAllocator( allocator )
        , stringLength( 0 )
        , stringCapacity( 0 )
        , allocatedMemory( nullptr )
    {

    }

    string::string( string& str )
    {
        *this = str;
    }

    string& string::operator = ( string& str )
    {
        stringAllocator = str.stringAllocator;
        stringLength = str.stringLength;
        stringCapacity = str.stringCapacity;
        allocatedMemory = str.allocatedMemory;

        return *this;
    }

    string::~string()
    {

    }

    const fnChar_t* string::c_str() const
    {
        return rawCharacters;
    }

    const fnString_t string::toStdString() const
    {
        return fnString_t( rawCharacters );
    }

    void string::reserve( const std::size_t size )
    {
        // Don't forget the null terminator
        stringCapacity = ( size + 1 );

        allocatedMemory = stringAllocator->allocate( stringCapacity * sizeof( fnChar_t ) );
    }

    void string::append( const fnChar_t character )
    {
        // Grow string allocation
        if ( stringLength == stringCapacity ) {
            allocatedMemory = stringAllocator->allocate( stringCapacity * sizeof( fnChar_t ) );
        }

        rawCharacters[stringLength++] = character;
    }

    void string::clear()
    {
        stringLength = 0;
    }

    const std::size_t string::length() const
    {
        return stringLength;
    }

    const std::size_t string::size() const
    {
        return stringLength * sizeof( fnChar_t );
    }

    const std::size_t string::capacity() const
    {
        return stringCapacity;
    }

    string& string::operator += ( const fnChar_t character )
    {
        append( character );
        return *this;
    }
}
