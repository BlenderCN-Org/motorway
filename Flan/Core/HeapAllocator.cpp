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
#include "HeapAllocator.h"

#include "MemoryAlignementHelpers.h"

static constexpr int DEBUG_MARKER = 0xEE;

Heap::Heap( const std::size_t heapSize, const std::size_t heapAlignement )
    : baseAddress( nullptr )
    , size( heapSize )
    , head( nullptr )
{
    baseAddress = malloc( heapSize );

#if FLAN_DEVBUILD
    memset( baseAddress, DEBUG_MARKER, heapSize );
#endif

    head = static_cast< Heap::AllocationHeader* >( baseAddress );
    head->previousAllocation = nullptr;
}

Heap::~Heap()
{
    free( baseAddress );
    size = 0;
    head = nullptr;
}

void* Heap::allocate( const std::size_t size )
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

    auto allocatedAddress = static_cast< void* >( head );

    head = static_cast< Heap::AllocationHeader* >( static_cast< Heap::AllocationHeader* >( allocatedAddress ) + sizeof( AllocationHeader ) ) + size;
    head->previousAllocation = static_cast< Heap::AllocationHeader* >( allocatedAddress );
    head->isAvailable = 0;

    return static_cast< void* >( static_cast< Heap::AllocationHeader* >( allocatedAddress ) + sizeof( AllocationHeader ) );
}

void Heap::free( void* allocatedMemory )
{
    Heap::AllocationHeader* iterator = head;

    while ( iterator != nullptr ) {
        if ( allocatedMemory == iterator ) {
            iterator->isAvailable = 1;
            return;
        }

        iterator = iterator->previousAllocation;
    }
}
