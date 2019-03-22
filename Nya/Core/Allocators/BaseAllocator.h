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

#include <cstdint>
#include <utility>

namespace
{
    template<typename T>
    constexpr std::uint8_t GetAllocatedArrayHeaderSize()
    {
        std::uint8_t headerSize = sizeof( std::size_t ) / sizeof( T );

        if ( sizeof( std::size_t ) % sizeof( T ) > 0ull ) {
            headerSize++;
        }

        return headerSize;
    }
}

class BaseAllocator
{
public:
                        BaseAllocator( const std::size_t size, void* baseAddress );
    virtual             ~BaseAllocator();

    void*               getBaseAddress() const;
    std::size_t         getSize() const;
    std::size_t         getMemoryUsage() const;
    std::size_t         getAllocationCount() const;

    virtual void*       allocate( const std::size_t allocationSize, const std::uint8_t alignment = 4 ) = 0;
    virtual void        free( void* pointer ) = 0;

protected:
    void*               baseAddress;
    std::size_t         memorySize;

    std::size_t         memoryUsage;
    std::size_t         allocationCount;
};

namespace nya
{
    namespace core
    {
        template<typename T, typename... TArgs>
        T* allocate( BaseAllocator* allocator, TArgs... args )
        {
            return new ( allocator->allocate( sizeof( T ), alignof( T ) ) ) T( std::forward<TArgs>( args )... );
        }

        template<typename T, typename... TArgs>
        T* allocateArray( BaseAllocator* allocator, const std::size_t arrayLength, TArgs... args )
        {
            constexpr std::uint8_t headerSize = GetAllocatedArrayHeaderSize<T>();

            T* allocationStart = static_cast<T*>( allocator->allocate( sizeof( T ) * ( arrayLength + headerSize ), alignof( T ) ) ) + headerSize;

            // Write array length at the begining of the allocation
            *( reinterpret_cast<std::size_t*>( allocationStart ) - 1 ) = arrayLength;

            for ( std::size_t allocation = 0; allocation < arrayLength; allocation++ ) {
                new ( allocationStart + allocation ) T( std::forward<TArgs>( args )... );
            }

            return allocationStart;
        }

        template<typename T>
        void free( BaseAllocator* allocator, T* object )
        {
            object->~T();
            allocator->free( object );
        }

        template<typename T>
        void freeArray( BaseAllocator* allocator, T* arrayObject )
        {
            std::size_t arrayLength = *( reinterpret_cast<std::size_t*>( arrayObject ) - 1ull );

            for ( std::size_t allocation = 0ull; allocation < arrayLength; allocation++ ) {
                ( arrayObject + allocation )->~T();
            }

            constexpr std::uint8_t headerSize = GetAllocatedArrayHeaderSize<T>();

            allocator->free( arrayObject - headerSize );
        }
    }
}
