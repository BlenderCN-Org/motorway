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
#include "GrowingStackAllocator.h"

#include "AllocationHelpers.h"

inline size_t RoundUpToMultiple( const size_t value, const size_t multiple )
{
    return ( value + multiple - 1 ) & ~( multiple - 1 );
}

GrowingStackAllocator::GrowingStackAllocator( const std::size_t maxSize, void* baseVirtualAddress, const std::size_t pageSize )
    : BaseAllocator( maxSize, baseVirtualAddress )
    , pageSize( pageSize )
    , endVirtualAddress( static_cast<void*>( (uint8_t*)baseVirtualAddress + maxSize ) )
    , currentPosition( baseVirtualAddress )
    , currentEndPosition( baseVirtualAddress )
    , previousPosition( nullptr )
{

}

GrowingStackAllocator::~GrowingStackAllocator()
{
    currentPosition = nullptr;
    previousPosition = nullptr;
}

void* GrowingStackAllocator::allocate( const std::size_t allocationSize, const std::uint8_t alignment )
{
    const auto adjustment = nya::core::AlignForwardAdjustmentWithHeader( currentPosition, alignment, sizeof( AllocationHeader ) );

    if ( memoryUsage + adjustment + allocationSize > memorySize ) {
        return nullptr;
    }

    std::uint8_t* allocatedAddress = static_cast< std::uint8_t* >( currentPosition ) + adjustment;

    // If we run out of pages, commit previously reserved pages
    if ( ( allocatedAddress + allocationSize ) > currentEndPosition ) {
        const std::size_t neededPhysicalSize = RoundUpToMultiple( adjustment + allocationSize, pageSize );

        // If we dont have any page left, return null and enjoy your crash!
        if ( ( static_cast<uint8_t*>( currentEndPosition ) + neededPhysicalSize ) > endVirtualAddress ) {
            return nullptr;
        }

        nya::core::VirtualAlloc( neededPhysicalSize, currentEndPosition );
        *(size_t*)currentEndPosition += neededPhysicalSize;
    }

    AllocationHeader* header = ( AllocationHeader* )( allocatedAddress - sizeof( AllocationHeader ) );
    header->adjustment = adjustment;
    header->previousAllocation = previousPosition;
    previousPosition = allocatedAddress;

    currentPosition = static_cast< void* >( allocatedAddress + allocationSize );

    memoryUsage += ( allocationSize + adjustment );
    allocationCount++;

    return static_cast< void* >( allocatedAddress );
}

void GrowingStackAllocator::free( void* pointer )
{
    std::uint8_t* memoryPointer = static_cast< std::uint8_t* >( pointer );

    // Retrieve Header
    AllocationHeader* header = ( AllocationHeader* )( memoryPointer - sizeof( AllocationHeader ) );

    memoryUsage -= ( std::uint8_t* )currentPosition - ( std::uint8_t* )pointer + header->adjustment;
    allocationCount--;

    currentPosition = memoryPointer - header->adjustment;
    previousPosition = header->previousAllocation;
}

void GrowingStackAllocator::clear()
{

}
