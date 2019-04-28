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
#include "TransactionHandler.h"

TransactionHandler::TransactionHandler( BaseAllocator* allocator )
    : cmdAllocator( allocator )
    , commandIdx( -1 )
    , commandCount( 0 )
    , commands{ nullptr }
{

}

TransactionHandler::~TransactionHandler()
{
    for ( int i = 0; i < commandCount; i++ ) {
        nya::core::free( cmdAllocator, commands[i] );
    }
}

void TransactionHandler::undo()
{
    if ( commandIdx < 0 ) {
        return;
    }

    commands[commandIdx]->undo();
    commandIdx--;
}

void TransactionHandler::redo()
{
    if ( ( commandIdx + 1 ) >= commandCount ) {
        return;
    }

    commands[commandIdx + 1]->execute();
    commandIdx++;
}

const std::string& TransactionHandler::getPreviousActionName() const
{
    return commands[commandIdx - 1]->getActionInfos();
}

const std::string& TransactionHandler::getNextActionName() const
{
    return commands[commandIdx + 1]->getActionInfos();
}
