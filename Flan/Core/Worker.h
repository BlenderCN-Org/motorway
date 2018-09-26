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

#include <thread>

#include "TaskManager.h"

struct Task;

class Worker
{
public:
                                Worker( TaskManager* activeTaskManager );
                                Worker( Worker& ) = delete;
                                Worker& operator = ( Worker& ) = delete;
                                ~Worker();

private:
    TaskManager*                taskManagerInstance;
    Task*                       currentTask;
    std::thread                 workerThread;

private:
    void                        work();
};
