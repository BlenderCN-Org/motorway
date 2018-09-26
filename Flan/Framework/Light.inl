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
#include "Light.h"

template<typename Data>
Light<Data>::Light()
    : data{}
    , isGPUDirty( true )
    , renderKey( 0 )
{

}

template<typename Data>
Light<Data>::Light( Data&& lightData )
    : data( std::move( lightData ) )
    , isGPUDirty( true )
    , renderKey( 0 )
{

}

template<typename Data>
Light<Data>::Light( FileSystemObject* stream )
    : isGPUDirty( true )
    , renderKey( 0 )
{
    restore( stream );
}

template<typename Data>
Light<Data>::Light( Light<Data>& light )
    : data( light.data )
    , isGPUDirty( true )
    , renderKey( 0 )
{

}

template<typename Data>
Light<Data>::~Light()
{
    data = {};
    isGPUDirty = false;
    renderKey = 0;
}

template<typename Data>
void Light<Data>::restore( FileSystemObject* stream )
{
    stream->read( reinterpret_cast< uint8_t* >( &data ), sizeof( Data ) );
}

template<typename Data>
void Light<Data>::save( FileSystemObject* stream )
{
    stream->write( reinterpret_cast< uint8_t* >( &data ), sizeof( Data ) );
}

template<typename Data>
void Light<Data>::setRenderKey( const fnRenderKey_t newRenderKey )
{
    renderKey = newRenderKey;
}

template<typename Data>
const fnRenderKey_t& Light<Data>::getRenderKey() const
{
    return renderKey;
}

template<typename Data>
Data& Light<Data>::getLightData()
{
    return data;
}

template<typename Data>
const Data* Light<Data>::getLightData() const
{
    return &data;
}
