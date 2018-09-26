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
#include "Worker.h"

#include "Task.h"

#include <mutex>
#include <algorithm>

Worker::Worker( TaskManager* activeTaskManager )
    : taskManagerInstance( activeTaskManager )
    , currentTask( nullptr )
    , workerThread( &Worker::work, this )

{

}

Worker::~Worker()
{
    if ( workerThread.joinable() ) {
        workerThread.join();
    }

    currentTask = nullptr;
    taskManagerInstance = nullptr;
}

void Worker::work()
{
    while ( 1 ) {
        {
            std::unique_lock<std::mutex> workerLock = std::unique_lock<std::mutex>( taskManagerInstance->workersMutex );

            while ( !taskManagerInstance->stopSignal && taskManagerInstance->taskList.empty() ) {
                taskManagerInstance->hasWorkTodo.wait( workerLock );
            }

            if ( taskManagerInstance->stopSignal ) {
                break;
            }

            currentTask = taskManagerInstance->taskList.back();
            taskManagerInstance->taskList.pop_back();
        }

        currentTask->Job();
        
        {
            std::unique_lock<std::mutex> pendingTaskLock = std::unique_lock<std::mutex>( taskManagerInstance->pendingTaskMutex );
            taskManagerInstance->pendingTasks.erase( std::find( taskManagerInstance->pendingTasks.begin(), taskManagerInstance->pendingTasks.end(), currentTask->ID ) );
        }

        delete currentTask;
        currentTask = nullptr;
    }
}
