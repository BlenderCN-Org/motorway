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

#include "BaseAllocator.h"

class GrowingStackAllocator final : public BaseAllocator
{
public:
            GrowingStackAllocator( const std::size_t maxSize, void* baseVirtualAddress, const std::size_t pageSize = 4096 );
            GrowingStackAllocator( GrowingStackAllocator& ) = delete;
            GrowingStackAllocator& operator = ( GrowingStackAllocator& ) = delete;
            ~GrowingStackAllocator();

    void*   allocate( const std::size_t allocationSize, const std::uint8_t alignment = 4 ) override;
    void    free( void* pointer ) override;
    void    clear();

private:
    struct AllocationHeader {
        void* previousAllocation;
        std::uint8_t adjustment;
    };

private:
    std::size_t pageSize;
    void*       endVirtualAddress;

    void*   currentPosition;
    void*   currentEndPosition;
    void*   previousPosition;
};
