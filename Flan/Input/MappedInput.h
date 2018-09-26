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

#include "InputKeys.h"
#include "InputAxis.h"
#include <map>
#include <set>

struct MappedInput
{
    std::set<fnStringHash_t>          Actions;
    std::set<fnStringHash_t>          States;
    std::map<fnStringHash_t, double>  Ranges;

    // Consumption helpers
    void eatAction( const fnStringHash_t action )
    {
        Actions.erase( action );
    }

    void eatState( const fnStringHash_t state )
    {
        States.erase( state );
    }

    void eatRange( const fnStringHash_t range )
    {
        auto iter = Ranges.find( range );

        if ( iter != Ranges.end() ) {
            Ranges.erase( iter );
        }
    }
};
