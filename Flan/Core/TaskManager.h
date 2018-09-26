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

#include "Task.h"

#include <list>
#include <vector>
#include <atomic>
#include <condition_variable>

class Worker;
class TaskManager
{
public:
                TaskManager();
                TaskManager( TaskManager& ) = delete;
                TaskManager& operator = ( TaskManager& ) = delete;
                ~TaskManager();

    void        create( const int overrideWorkerCount = -1 );
    fnTaskId_t  addTask( Task&& task );
    fnTaskId_t  addTask( fnTaskFunc_t jobToComplete );
    bool        isTaskFinished( const fnTaskId_t taskID );

private:
    friend class Worker;

    std::list<Task*>        taskList;
    std::vector<Worker*>    workers;
    std::vector<fnTaskId_t> pendingTasks;
    std::atomic_bool        stopSignal;
    std::condition_variable hasWorkTodo;
    std::mutex              workersMutex;
    std::mutex              pendingTaskMutex;

private:
    fnTaskId_t              dispatchAndNotify( Task* taskToBeCompleted );
};
