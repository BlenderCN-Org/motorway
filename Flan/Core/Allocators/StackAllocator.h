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

#include "BaseAllocator.h"

class StackAllocator final : public BaseAllocator
{
public:
            StackAllocator( const std::size_t size, void* baseAddress );
            StackAllocator( StackAllocator& ) = delete;
            StackAllocator& operator = ( StackAllocator& ) = delete;
            ~StackAllocator();

    void*   allocate( const std::size_t allocationSize, const std::uint8_t alignment = 4 ) override;
    void    free( void* pointer ) override;

private:
    struct AllocationHeader {
        void* previousAllocation;
        std::uint8_t adjustment;
    };

private:
    void*   currentPosition;
    void*   previousPosition;
};
