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

static constexpr int DEBUG_MARKER = 0xEE;

template<typename T>
Pool<T>::Pool( const std::size_t poolCapacity, const std::size_t heapAlignement, Heap* heapAllocator )
    : capacity( poolCapacity )
    , allocationIndex( 0 )
{
    // Memory layout
    //
    // [ Bitfield, defining pool member availability ]
    // [ Memory ]
    const int freeFlagsSize = static_cast<int>( std::ceil( poolCapacity / 32 ) ) * sizeof( int );

    std::uint32_t allocationSize = static_cast<std::uint32_t>( freeFlagsSize + poolCapacity * sizeof( T ) );
    if ( heapAllocator != nullptr ) {
        baseAddress = heapAllocator->allocate( allocationSize );
    } else {
        baseAddress = malloc( allocationSize );
    }

#if FLAN_DEVBUILD
    // Reset bifield(s)
    memset( baseAddress, 0, freeFlagsSize );
    memset( static_cast<int*>( baseAddress ) + freeFlagsSize, DEBUG_MARKER, poolCapacity * sizeof( T ) );
#endif
}

void UpdateAllocationBitfield( void* baseAddress, const std::size_t allocationIndex )
{
    const int bitfieldIndex = static_cast<int>( std::ceil( allocationIndex / 32.0f ) );

    int* allocationIndexes = static_cast<int*>( baseAddress ) + ( bitfieldIndex * sizeof( int ) );

    const int bitIndex = static_cast<int>( allocationIndex - ( bitfieldIndex * 32 ) );
    *allocationIndexes ^= 1 << bitIndex;
}

template<typename T>
Pool<T>::~Pool()
{
    ::free( baseAddress );
}

template<typename T>
T* Pool<T>::get()
{
    UpdateAllocationBitfield( baseAddress, allocationIndex );
    allocationIndex++;

    return static_cast< T* >( baseAddress + ( sizeof( T ) * allocationIndex ) );
}

template<typename T>
void Pool<T>::release( T* poolAllocation )
{
    const int allocatedIndex = ( poolAllocation - baseAddress ) / sizeof( T );

    UpdateAllocationBitfield( baseAddress, allocatedIndex );
}

template<typename T>
const std::size_t Pool<T>::getAllocationCount() const
{
    return allocationCount;
}

template<typename T>
const std::size_t Pool<T>::getMemoryUsage() const
{
    return memoryUsage;
}
