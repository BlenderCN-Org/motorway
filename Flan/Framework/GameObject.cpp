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

#include "Shared.h"
#include "GameObject.h"

#include <Core/Factory.h>

GameObject::GameObject( const fnStringHash_t objectNameHashcode )
    : objectHashcode( objectNameHashcode )
{

}

void GameObject::Update( const float frameTime )
{

}

const fnStringHash_t GameObject::getHashcode() const
{
    return objectHashcode;
}

bool GameObject::isHashcodeEqual( const fnStringHash_t hashcodeToCompareTo )
{
    return ( objectHashcode == hashcodeToCompareTo );
}

static constexpr uint32_t GOBJ = 0x4A424F47;

void GameObject::Serialize( FileSystemObject* stream )
{
    // GameObject Binary Header
    //  GOBJ Magic          4
    //  Object Hashcode     4
    //  __PADDING__         8

    stream->write<uint32_t>( GOBJ );
    stream->write<fnStringHash_t>( objectHashcode );
    stream->writePadding();
}

void GameObject::Deserialize( FileSystemObject* stream )
{
    uint32_t magicHeader = 0;
    stream->read( (uint8_t*)&magicHeader, sizeof( uint32_t ) );

    if ( magicHeader != GOBJ ) {
        FLAN_CERR << "Not a GameObject binary (bad magic)" << std::endl;
        return;
    }

    stream->read( (uint8_t*)&objectHashcode, sizeof( uint32_t ) );
    
    // Skip padding
    stream->skip( 8 );
}
