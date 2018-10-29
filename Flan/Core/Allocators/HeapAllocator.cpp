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
#include <Shared.h>
#include "HeapAllocator.h"

#include <Core/MemoryAlignementHelpers.h>

static constexpr int DEBUG_MARKER = 0xEE;

Heap::Heap( const std::size_t heapSize, const std::size_t heapAlignement, Heap* allocator )
    : memoryAllocator( allocator )
    , baseAddress( nullptr )
    , size( heapSize )
    , head( nullptr )
    , allocationCount( 0 )
    , memoryUsage( 0 )
{
    baseAddress = ( allocator == nullptr ) ? malloc( heapSize ) : allocator->allocate( heapSize );

#if FLAN_DEVBUILD
    memset( baseAddress, DEBUG_MARKER, heapSize );
#endif

    head = static_cast< Heap::AllocationHeader* >( baseAddress );
    head->previousAllocation = nullptr;
}

Heap::~Heap()
{
    if ( memoryAllocator == nullptr ) {
        ::free( baseAddress );
    } else {
        memoryAllocator->free( baseAddress );
    }

    memoryAllocator = nullptr;
    baseAddress = nullptr;
    size = 0;
    head = nullptr;
}

void* Heap::allocate( const std::uint32_t size )
{
    // Try to reuse
    Heap::AllocationHeader* iterator = head;

    while ( iterator != nullptr ) {
        if ( iterator->isAvailable == 1 && iterator->allocationSize >= size ) {
            iterator->isAvailable = 0;
            return static_cast< void* >( static_cast< Heap::AllocationHeader* >( iterator ) + sizeof( AllocationHeader ) );
        }

        iterator = iterator->previousAllocation;
    }

    head->allocationSize = size;
    head->isAvailable = 0;

    Heap::AllocationHeader* allocatedAddress = static_cast< Heap::AllocationHeader* >( head + sizeof( AllocationHeader ) );

    head = allocatedAddress + size;
    head->previousAllocation = static_cast< Heap::AllocationHeader* >( allocatedAddress - sizeof( AllocationHeader ) );

    memoryUsage += size;
    allocationCount++;

    return static_cast< void* >( allocatedAddress );
}

void Heap::free( void* allocatedMemory )
{
    Heap::AllocationHeader* iterator = head;

    while ( iterator != nullptr ) {
        if ( allocatedMemory == iterator ) {
            iterator->isAvailable = 1;
            memoryUsage -= size;
            allocationCount--;
            return;
        }

        iterator = iterator->previousAllocation;
    }
}

const std::size_t Heap::getAllocationCount() const
{
    return allocationCount;
}

const std::size_t Heap::getMemoryUsage() const
{
    return memoryUsage;
}
