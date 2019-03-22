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

#if NYA_UNIX

#include <stdlib.h>
#include <cstdint>
#include <sys/mman.h>

namespace nya
{
    namespace core
    {
        static void* malloc( const std::size_t size )
        {
            return ::malloc( size );
        }

        static void* realloc( void* block, const std::size_t size )
        {
            return ::realloc( block, size );
        }

        static void free( void* block )
        {
            ::free( block );
        }

#if NYA_GCC
        static void* AlignedMalloc( const std::size_t size, const std::uint8_t alignment )
        {
            return ::aligned_alloc( size, alignment );
        }

        static void* AlignedRealloc( void* block, const std::size_t size, const std::uint8_t alignment )
        {
            void* reallocatedBlock = realloc( block, size );
            ::posix_memalign( &reallocatedBlock, size, alignment );

            return reallocatedBlock;
        }

        static void FreeAligned( void* block )
        {
            ::free( block );
        }
#endif

        static void* ReserveAddressSpace( const std::size_t reservationSize, void* startAddress = nullptr )
        {
            return ::mmap( startAddress, reservationSize, PROT_NONE, MAP_SHARED | MAP_GROWSDOWN, -1, 0 );
        }

        static void ReleaseAddressSpace( void* address )
        {
            // TODO munmap needs to know the size of the allocation...
            // ::munmap( address, 0  );
        }

        static void* VirtualAlloc( const std::size_t allocationSize, void* reservedAddressSpace )
        {
            return nullptr;
        }

        static void VirtualFree( void* allocatedAddress )
        {

        }

        static std::size_t GetPageSize()
        {
            return 0ull;
        }

        static void* PageAlloc( const std::size_t size )
        {
            return nullptr;
        }
    }
}
#endif
