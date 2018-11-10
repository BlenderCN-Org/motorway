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

class BaseAllocator;

namespace flan
{
    class string
    {
    public:
                        string( BaseAllocator* allocator = nullptr );
                        string( string& str );
                        string& operator = ( string& str );
                        ~string();

    const fnChar_t*     c_str() const;
    const fnString_t    toStdString() const;

    void                reserve( const std::size_t size );
    void                append( const fnChar_t character );
    void                clear();

    const std::size_t   length() const;
    const std::size_t   size() const;
    const std::size_t   capacity() const;

    string&             operator += ( const fnChar_t character );

    private:
        BaseAllocator*  stringAllocator;

        std::size_t     stringLength;
        std::size_t     stringCapacity;

        union
        {
            void*        allocatedMemory;
            fnChar_t*    rawCharacters;
        };
    };
}

