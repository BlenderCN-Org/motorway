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
#include "TaskManager.h"

#include "Environment.h"
#include "Worker.h"

#include "Allocators/HeapAllocator.h"

TaskManager::TaskManager()
    : memoryAllocator( nullptr )
    , stopSignal( false )
{

}

TaskManager::~TaskManager()
{
    stopSignal = true;

    hasWorkTodo.notify_all();

    for ( Worker* worker : workers ) {
        if ( worker != nullptr ) {
            memoryAllocator->free( worker );
            worker = nullptr;
        }
    }
    workers.clear();

    for ( Task* task : taskList ) {
        if ( task != nullptr ) {
            memoryAllocator->free( task );
            task = nullptr;
        }
    }

    taskList.clear();
    stopSignal = false;

    pendingTasks.clear();

    memoryAllocator = nullptr;
}

void TaskManager::create( Heap* allocator, const int overrideWorkerCount )
{
    memoryAllocator = allocator;

    auto cpuCoreCount = ( overrideWorkerCount != -1 ) ? overrideWorkerCount : flan::core::GetCPUCoreCount();
    for ( int i = 0; i < cpuCoreCount; ++i ) {
        workers.push_back( allocator->allocate<Worker>( this ) );
    }

    FLAN_CLOG << "Created TaskManager with " << cpuCoreCount << " workers" << std::endl;
}

fnTaskId_t TaskManager::addTask( Task&& task )
{
    return dispatchAndNotify( memoryAllocator->allocate<Task>( std::move( task ) ) );
}

fnTaskId_t TaskManager::addTask( fnTaskFunc_t jobToComplete )
{
    Task* task = memoryAllocator->allocate<Task>();
    task->Job = jobToComplete; 

    return dispatchAndNotify( task );
}

bool TaskManager::isTaskFinished( const fnTaskId_t taskID )
{
    bool isFinished = true;

    {
        std::unique_lock<std::mutex> pendingTaskLock = std::unique_lock<std::mutex>( pendingTaskMutex );
        isFinished = ( std::find( pendingTasks.begin(), pendingTasks.end(), taskID ) == pendingTasks.end() );
    }

    return isFinished;
}

fnTaskId_t TaskManager::dispatchAndNotify( Task* taskToBeCompleted )
{
    static fnTaskId_t g_TaskGlobalIndex = 0;

    {
        std::unique_lock<std::mutex> workerLock( workersMutex );

        taskToBeCompleted->ID = g_TaskGlobalIndex++;

        taskList.push_back( taskToBeCompleted );
        taskList.sort();
    }

    hasWorkTodo.notify_one();
    
    {
        std::unique_lock<std::mutex> pendingTaskLock = std::unique_lock<std::mutex>( pendingTaskMutex );
        pendingTasks.push_back( taskToBeCompleted->ID );
    }

    return taskToBeCompleted->ID;
}
