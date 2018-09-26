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

TransactionHandler::TransactionHandler()
    : commandIdx( -1 )
    , commandCount( 0 )
    , commands( 64, nullptr )
{

}

TransactionHandler::~TransactionHandler()
{

}

void TransactionHandler::commit( TransactionCommand* cmd )
{
    commandCount++;
    commandIdx++;

    if ( commandCount > commands.size() ) {
        commands.push_back( nullptr );
    }

    cmd->execute();

    // If we are in the past, clear the future commits
    if ( commandCount - commandIdx > 1 ) {
        for ( int i = commandIdx; i < commandCount; i++ ) {
            if ( commands[i] != nullptr ) {
                delete commands[i];
                commands[i] = nullptr;
            }
        }

        commandCount -= ( commandCount - commandIdx );
    }

    commands[commandIdx] = cmd;
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

    commandIdx++;
    commands[commandIdx + 1]->execute();
}

const std::string& TransactionHandler::getPreviousActionName() const
{
    return commands[commandIdx - 1]->getActionInfos();
}

const std::string& TransactionHandler::getNextActionName() const
{
    return commands[commandIdx + 1]->getActionInfos();
}
