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
#include "LinearAllocator.h"

#include "AllocationHelpers.h"

LinearAllocator::LinearAllocator( const std::size_t size, void* baseAddress )
    : BaseAllocator( size, baseAddress )
    , currentPosition( baseAddress )
{

}

LinearAllocator::~LinearAllocator()
{
    currentPosition = nullptr;
}

void* LinearAllocator::allocate( const std::size_t allocationSize, const std::uint8_t alignment )
{
    const auto adjustment = flan::core::AlignForwardAdjustment( currentPosition, alignment );

    if ( memoryUsage + adjustment + allocationSize > memorySize ) {
        return nullptr;
    }

    std::uint8_t* allocatedAddress = static_cast< std::uint8_t* >( currentPosition ) + adjustment;
    currentPosition = static_cast< void* >( allocatedAddress + allocationSize );

    memoryUsage += ( allocationSize + adjustment );
    allocationCount++;

    return static_cast< void* >( allocatedAddress );
}

void LinearAllocator::free( void* pointer )
{

}

void LinearAllocator::clear()
{
    allocationCount = 0;
    memoryUsage = 0;
    currentPosition = baseAddress;
}
