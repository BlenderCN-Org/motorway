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
#include <Shared.h>
#include "PoolAllocator.h"

#include "AllocationHelpers.h"

PoolAllocator::PoolAllocator( const std::size_t objectSize, const std::uint8_t objectAlignment, const std::size_t size, void* baseAddress )
    : BaseAllocator( size, baseAddress )
    , objectSize( objectSize )
    , objectAlignment( objectAlignment )
    , freeList( nullptr )
{
    clear();
}

PoolAllocator::~PoolAllocator()
{
    freeList = nullptr;
}

void* PoolAllocator::allocate( const std::size_t allocationSize, const std::uint8_t alignment )
{
    void* p = freeList;
    freeList = ( void** )( *freeList );
    memoryUsage += allocationSize;
    allocationCount++;
    return p;
}

void PoolAllocator::free( void* pointer )
{
    *( ( void** )pointer ) = freeList;
    freeList = ( void** )pointer;
    memoryUsage -= objectSize;
    allocationCount--;
}

void PoolAllocator::clear()
{
    const auto adjustment = nya::core::AlignForwardAdjustment( baseAddress, objectAlignment );

    freeList = ( void** )( ( std::uint8_t* )baseAddress + adjustment );
    size_t numObjects = ( memorySize - adjustment ) / objectSize;
    void** p = freeList;

    //Initialize free blocks list 
    for ( size_t i = 0; i < numObjects - 1; i++ ) {
        *p = ( std::uint8_t* )p + objectSize;
        p = ( void** )*p;
    }

    *p = nullptr;
    allocationCount = 0;
}