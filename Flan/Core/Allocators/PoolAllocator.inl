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

static constexpr int DEBUG_MARKER = 0xEE;

template<typename T>
Pool<T>::Pool( const std::size_t poolCapacity, const std::size_t heapAlignement )
    : capacity( poolCapacity )
    , allocationIndex( 0 )
{
    baseAddress = malloc( poolCapacity * sizeof( T ) );

#if FLAN_DEVBUILD
    memset( baseAddress, DEBUG_MARKER, poolCapacity * sizeof( T ) );
#endif
}

template<typename T>
Pool<T>::~Pool()
{
    ::free( baseAddress );
}

template<typename T>
T* Pool<T>::get()
{
    return static_cast< T* >( baseAddress + ( sizeof( T ) * allocationIndex++ ) );
}

template<typename T>
void Pool<T>::release( T* poolAllocation )
{
    //return static_cast< T* >( baseAddress + ( sizeof( T ) * allocationIndex++ ) );
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
