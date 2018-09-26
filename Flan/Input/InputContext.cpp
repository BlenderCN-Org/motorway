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
#include "InputContext.h"

using namespace flan::core;

InputContext::InputContext()
    : rangeConverter( new RangeConverter() )
{

}

InputContext::~InputContext()
{
    actionMap.clear();
    stateMap.clear();
    rangeMap.clear();
    sensitivityMap.clear();
}

void InputContext::bindActionToButton( eInputKey button, fnStringHash_t action )
{
    auto iter = actionMap.find( button );
    if ( iter == actionMap.end() ) {
        actionMap.emplace( button, action );
    } else {
        actionMap[button] = action;
    }
}

void InputContext::bindStateToButton( eInputKey button, fnStringHash_t state )
{
    auto iter = stateMap.find( button );
    if ( iter == stateMap.end() ) {
        stateMap.emplace( button, state );
    } else {
        stateMap[button] = state;
    }
}

void InputContext::bindRangeToAxis( eInputAxis axis, fnStringHash_t range )
{
    auto iter = rangeMap.find( axis );
    if ( iter == rangeMap.end() ) {
        rangeMap.emplace( axis, range );
    } else {
        rangeMap[axis] = range;
    }
}

void InputContext::bindSensitivityToRange( double sensitivity, fnStringHash_t range )
{
    auto iter = sensitivityMap.find( range );
    if ( iter == sensitivityMap.end() ) {
        sensitivityMap.emplace( range, sensitivity );
    } else {
        sensitivityMap[range] = sensitivity;
    }
}

void InputContext::setRangeDataRange( double inputMin, double inputMax, double outputMin, double outputMax, fnStringHash_t range )
{
    rangeConverter->addRangeToConverter( range, inputMin, inputMax, outputMin, outputMax );
}

bool InputContext::mapButtonToAction( eInputKey button, fnStringHash_t& out ) const
{
    auto iter = actionMap.find( button );
    if ( iter == actionMap.end() ) {
        return false;
    }

    out = iter->second;
    return true;
}

bool InputContext::mapButtonToState( eInputKey button, fnStringHash_t& out ) const
{
    auto iter = stateMap.find( button );
    if ( iter == stateMap.end() ) {
        return false;
    }

    out = iter->second;
    return true;
}

bool InputContext::mapAxisToRange( eInputAxis axis, fnStringHash_t& out ) const
{
    auto iter = rangeMap.find( axis );
    if ( iter == rangeMap.end() ) {
        return false;
    }

    out = iter->second;
    return true;
}

double InputContext::getSensitivity( fnStringHash_t range ) const
{
    auto iter = sensitivityMap.find( range );
    if ( iter == sensitivityMap.end() ) {
        return 1.0;
    }

    return iter->second;
}
