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

#if NYA_WIN
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

#if NYA_MSVC
        static void* AlignedMalloc( const std::size_t size, const std::uint8_t alignment )
        {
            return ::_aligned_malloc( size, alignment );
        }

        static void* AlignedRealloc( void* block, const std::size_t size, const std::uint8_t alignment )
        {
            return ::_aligned_realloc( block, size, alignment );
        }

        static void FreeAligned( void* block )
        {
            ::_aligned_free( block );
        }
#endif

        static void* ReserveAddressSpace( const std::size_t reservationSize, void* startAddress = nullptr )
        {
            return ::VirtualAlloc( startAddress, reservationSize, ( MEM_RESERVE | MEM_TOP_DOWN ), PAGE_NOACCESS );
        }

        static void ReleaseAddressSpace( void* address )
        {
            ::VirtualFree( address, 0, MEM_RELEASE );
        }

        static void* VirtualAlloc( const std::size_t allocationSize, void* reservedAddressSpace )
        {
            return ::VirtualAlloc( reservedAddressSpace, allocationSize, MEM_COMMIT, PAGE_READWRITE );
        }

        static void VirtualFree( void* allocatedAddress )
        {
            ::VirtualFree( allocatedAddress, 0, MEM_DECOMMIT );
        }

        static std::size_t GetPageSize()
        {
            SYSTEM_INFO systemInfos = {};
            GetSystemInfo( &systemInfos );

            return systemInfos.dwAllocationGranularity;
        }

        static void* PageAlloc( const std::size_t size )
        {
            return ::VirtualAlloc( nullptr, size, MEM_COMMIT, PAGE_READWRITE );
        }
    }
}
#endif
