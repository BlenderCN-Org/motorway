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

#if FLAN_WIN
namespace flan
{
    namespace core
    {
        void* MAlloc( const std::size_t size )
        {
            return ::malloc( size );
        }

        void* ReAlloc( void* block, const std::size_t size )
        {
            return ::realloc( block, size );
        }

        void Free( void* block )
        {
            ::free( block );
        }

#if FLAN_MSVC
        void* AlignedMAlloc( const std::size_t size, const std::uint8_t alignment )
        {
            return ::_aligned_malloc( size, alignment );
        }

        void* AlignedReAlloc( void* block, const std::size_t size, const std::uint8_t alignment )
        {
            return ::_aligned_realloc( block, size, alignment );
        }

        void FreeAligned( void* block )
        {
            ::_aligned_free( block );
        }
#endif

        void* PageAlloc( const std::size_t size )
        {
            return VirtualAlloc( nullptr, size, MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE );
        }

        void FreePage( void* page )
        {
            VirtualFree( page, 0, MEM_RELEASE );
        }
    }
}
#endif
