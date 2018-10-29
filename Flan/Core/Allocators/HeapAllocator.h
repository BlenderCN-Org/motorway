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

class Heap
{
public:
            Heap( const std::size_t heapSize, const std::size_t heapAlignement = 16, Heap* allocator = nullptr );
            Heap( Heap& ) = delete;
            Heap& operator = ( Heap& ) = delete;
            ~Heap();
            
    template<typename T, typename... TArgs>
    T* allocate( TArgs... args ) {
        return new ( allocate( sizeof( T ) ) ) T( std::forward<TArgs>( args )... );
    }

    void*   allocate( const std::uint32_t size );
    void    free( void* allocatedMemory );

    const std::size_t getAllocationCount() const;
    const std::size_t getMemoryUsage() const;

private:
    struct AllocationHeader {
        std::uint32_t           allocationSize;
        std::uint8_t            isAvailable;
        std::uint8_t            __PADDING__[3];
        Heap::AllocationHeader* previousAllocation;
    };
    FLAN_IS_MEMORY_ALIGNED( 16, AllocationHeader )

private:
    Heap*               memoryAllocator;
    void*               baseAddress;
    std::size_t         size;
    AllocationHeader*   head;

    std::size_t         allocationCount;
    std::size_t         memoryUsage;
};
