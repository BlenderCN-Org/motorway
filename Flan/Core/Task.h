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

#include <functional>

using fnTaskFunc_t = std::function<void( void )>;
using fnTaskId_t = unsigned int;

// Asynchronous task to be submitted to a TaskManager instance
struct Task
{
    Task()
        : ID( 0 )
        , Priority( DEFAULT )
    {

    }

    fnTaskId_t    ID;
    enum : unsigned int
    {
        LOWEST = 0,
        DEFAULT,
        HIGHEST
    } Priority;

    fnTaskFunc_t  Job;

    inline bool operator < ( Task* otherTask )
    {
        return Priority < otherTask->Priority;
    }
};
