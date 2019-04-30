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

#include "TransactionCommand.h"

#include <vector>

class TransactionHandler
{
public:
                                TransactionHandler( BaseAllocator* allocator );
                                TransactionHandler( TransactionHandler& ) = delete;
                                TransactionHandler& operator = ( TransactionHandler& ) = delete;
                                ~TransactionHandler();

    template<typename T, typename... TArgs>
    void commit( TArgs... args )
    {
        if ( commandCount >= MAX_HISTORY_SIZE ) {
            NYA_CERR << "Transaction Commands limit reached (check TransactionHandler::MAX_HISTORY_SIZE)" << std::endl;
            return;
        }

        commandCount++;
        commandIdx++;

        // If we are in the past, clear the future commits
        if ( commandCount - commandIdx > 1 ) {
            commandCount -= ( commandCount - commandIdx );
        }

        auto cmd = nya::core::allocate<T>( cmdAllocator, std::forward<TArgs>( args )... );
        cmd->execute();

        commands[commandIdx] = cmd;
    }

    void                        undo();
    void                        redo();

    const std::string&          getPreviousActionName() const;
    const std::string&          getNextActionName() const;

private:
    static constexpr int32_t MAX_HISTORY_SIZE = 4096;

private:
    BaseAllocator*                          cmdAllocator;
    TransactionCommand*                     commands[MAX_HISTORY_SIZE];

    int                                     commandIdx;
    int                                     commandCount;
};
