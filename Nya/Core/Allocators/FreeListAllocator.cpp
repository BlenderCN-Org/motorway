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
#include "FreeListAllocator.h"

#include "AllocationHelpers.h"

FreeListAllocator::FreeListAllocator( const std::size_t size, void* baseAddress )
    : BaseAllocator( size, baseAddress )
    , freeBlockList( static_cast<FreeBlock*>( baseAddress ) )
{
    freeBlockList->size = size;
    freeBlockList->next = nullptr;
}

FreeListAllocator::~FreeListAllocator()
{
    freeBlockList = nullptr;
}

void* FreeListAllocator::allocate( const std::size_t allocationSize, const std::uint8_t alignment )
{
    FreeBlock* previousFreeBlock = nullptr;
    FreeBlock* freeBlock = freeBlockList;

    while ( freeBlock != nullptr ) {
        const auto adjustment = nya::core::AlignForwardAdjustmentWithHeader( freeBlock, alignment, sizeof( AllocationHeader ) );
        size_t requiredSize = allocationSize + adjustment;

        // If allocation doesn't fit in this FreeBlock, try the next 
        if ( freeBlock->size < requiredSize ) {
            previousFreeBlock = freeBlock;
            freeBlock = freeBlock->next;
            continue;
        }

        // If allocations in the remaining memory will be impossible 
        if ( freeBlock->size - requiredSize <= sizeof( AllocationHeader ) ) {
            // Increase allocation size instead of creating a new FreeBlock 
            requiredSize = freeBlock->size;

            if ( previousFreeBlock != nullptr )
                previousFreeBlock->next = freeBlock->next;
            else
                freeBlock = freeBlock->next;
        } else {
            // Else create a new FreeBlock containing remaining memory 
            FreeBlock* nextFreeBlock = static_cast<FreeBlock*>( freeBlock + requiredSize );

            nextFreeBlock->size = freeBlock->size - requiredSize;
            nextFreeBlock->next = freeBlock->next;

            if ( previousFreeBlock != nullptr )
                previousFreeBlock->next = nextFreeBlock;
            else
                freeBlockList = nextFreeBlock;
        }

        std::uint8_t* allocatedAddress = reinterpret_cast< std::uint8_t* >( freeBlock ) + adjustment;
        AllocationHeader* header = reinterpret_cast<AllocationHeader*>( allocatedAddress - sizeof( AllocationHeader ) );
        header->size = requiredSize;
        header->adjustment = adjustment;

        memoryUsage += requiredSize;
        allocationCount++;

        return static_cast< void* >( allocatedAddress );
    }

    return nullptr;
}

void FreeListAllocator::free( void* pointer )
{
    AllocationHeader* header = ( reinterpret_cast<AllocationHeader*>( pointer ) - sizeof( AllocationHeader ) );

    const std::uint8_t* blockBaseAddress = reinterpret_cast<std::uint8_t*>( pointer ) - header->adjustment;
    const size_t blockSize = header->size;
    const std::uint8_t* blockEndAddress = blockBaseAddress + blockSize;

    FreeBlock* previousFreeBlock = nullptr;
    FreeBlock* freeBlock = freeBlockList;

    while ( freeBlock != nullptr ) {
        if ( reinterpret_cast<std::uint8_t*>(freeBlock) >= blockEndAddress ) {
            break;
        }

        previousFreeBlock = freeBlock;
        freeBlock = freeBlock->next;
    }

    if ( previousFreeBlock == nullptr ) {
        previousFreeBlock = ( FreeBlock* )blockBaseAddress;
        previousFreeBlock->size = blockSize;
        previousFreeBlock->next = freeBlockList;
        
        freeBlockList = previousFreeBlock;
    } else if ( ( std::uint8_t* )previousFreeBlock + previousFreeBlock->size == blockBaseAddress ) {
        previousFreeBlock->size += blockSize;
    } else {
        FreeBlock* temp = ( FreeBlock* )blockBaseAddress;
        temp->size = blockSize;
        temp->next = previousFreeBlock->next;
        previousFreeBlock->next = temp;
        previousFreeBlock = temp;
    }

    if ( freeBlock != nullptr && ( std::uint8_t* )freeBlock == blockEndAddress ) {
        previousFreeBlock->size += freeBlock->size;
        previousFreeBlock->next = freeBlock->next;
    }

    allocationCount--;
    memoryUsage -= blockSize;
}
