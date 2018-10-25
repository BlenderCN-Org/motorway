/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

template<typename T>
class Pool
{
public:
                        Pool( const std::size_t poolCapacity, const std::size_t heapAlignement = 16 );
                        Pool( Pool& ) = delete;
                        Pool& operator = ( Pool& ) = delete;
                        ~Pool();
           
    T*                  get();
    void                release( T* poolAllocation );

    const std::size_t   getAllocationCount() const;
    const std::size_t   getMemoryUsage() const;

private:
    void*               baseAddress;
    std::size_t         capacity;
    std::size_t         allocationIndex;

    std::size_t         allocationCount;
    std::size_t         memoryUsage;
};

#include "PoolAllocator.inl"
