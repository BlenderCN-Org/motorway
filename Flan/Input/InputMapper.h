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

#include "InputKeys.h"
#include "InputAxis.h"
#include "MappedInput.h"

#include <map>
#include <functional>
#include <list>

using InputCallback_t = std::function<void( MappedInput&, float )>;

class InputContext;

class InputMapper
{
public:
                                            InputMapper();
                                            InputMapper( InputMapper& ) = delete;
                                            ~InputMapper();

    void                                    update( const float frameTime );
    void                                    clear();
    void                                    setRawButtonState( flan::core::eInputKey button, bool pressed, bool previouslypressed );
    void                                    setRawAxisValue( flan::core::eInputAxis axis, double value );
    void                                    addCallback( InputCallback_t callback, int priority );
    void                                    addContext( const fnStringHash_t name, InputContext* context );

    void                                    pushContext( const fnStringHash_t name );
    void                                    popContext();

    void                                    deserialize( FileSystemObject* file );

private:
    std::map<fnStringHash_t, InputContext*> inputContexts;
    std::list<InputContext*>                activeContexts;
    std::multimap<int, InputCallback_t>     callbackTable;
    MappedInput                             currentMappedInput;

private:
    bool                                    mapButtonToAction( flan::core::eInputKey button, fnStringHash_t& action ) const;
    bool                                    mapButtonToState( flan::core::eInputKey button, fnStringHash_t& state ) const;
    void                                    mapAndEatButton( flan::core::eInputKey button );
};
