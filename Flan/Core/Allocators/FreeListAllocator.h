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

class FreeListAllocator final : public BaseAllocator
{
public:
            FreeListAllocator( const std::size_t size, void* baseAddress );
            FreeListAllocator( FreeListAllocator& ) = delete;
            FreeListAllocator& operator = ( FreeListAllocator& ) = delete;
            ~FreeListAllocator();

    void*   allocate( const std::size_t allocationSize, const std::uint8_t alignment = 4 ) override;
    void    free( void* pointer ) override;
    
private:
    struct AllocationHeader { 
        std::size_t     size;
        std::uint8_t    adjustment; 
    };

    struct FreeBlock { 
        std::size_t size; 
        FreeBlock*  next; 
    };

private:
    FreeBlock* freeBlockList;
};
