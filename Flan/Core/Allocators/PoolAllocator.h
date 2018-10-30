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

#include "BaseAllocator.h"

class PoolAllocator final : public BaseAllocator
{
public:
            PoolAllocator( const std::size_t objectSize, const std::uint8_t objectAlignment, const std::size_t size, void* baseAddress );
            PoolAllocator( PoolAllocator& ) = delete;
            PoolAllocator& operator = ( PoolAllocator& ) = delete;
            ~PoolAllocator();

    void*   allocate( const std::size_t allocationSize, const std::uint8_t alignment = 4 ) override;
    void    free( void* pointer ) override;
    void    clear();

private:
    const std::size_t   objectSize;
    const std::uint8_t  objectAlignment;
    void**              freeList;
};
