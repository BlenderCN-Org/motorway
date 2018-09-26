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
#include "AppShared.h"

#include <Core/FileLogger.h>
#include <Core/TaskManager.h>
#include <Physics/DynamicsWorld.h>
#include <Network/GameHost.h>

std::unique_ptr<FileLogger>              g_FileLogger( nullptr );
std::unique_ptr<TaskManager>             g_TaskManager( nullptr );
std::unique_ptr<DynamicsWorld>           g_DynamicsWorld( nullptr );
std::unique_ptr<GameHost>                g_GameHost( nullptr );
