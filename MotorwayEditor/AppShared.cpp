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
#include <FileSystem/VirtualFileSystem.h>
#include <FileSystem/FileSystemNative.h>
#include <Core/TaskManager.h>
#include <Display/DisplaySurface.h>
#include <Input/InputReader.h>
#include <Input/InputMapper.h>
#include <Rendering/RenderDevice.h>
#include <Graphics/WorldRenderer.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/ShaderStageManager.h>
#include <Graphics/DrawCommandBuilder.h>
#include <Graphics/RenderableEntityManager.h>
#include <Audio/AudioDevice.h>
#include <Physics/DynamicsWorld.h>
#include <Network/GameClient.h>
#include <Framework/Scene.h>

#if FLAN_DEVBUILD
#include "Core/FileSystem/FileSystemWatchdog.h"
#include <Framework/TransactionHandler/TransactionHandler.h>
#include <Physics/PhysicsDebugDraw.h>
#endif

FileLogger*              g_FileLogger( nullptr );
VirtualFileSystem*       g_VirtualFileSystem( nullptr );
TaskManager*             g_TaskManager( nullptr );
DisplaySurface*          g_MainDisplaySurface( nullptr );
InputReader*             g_InputReader( nullptr );
InputMapper*             g_InputMapper( nullptr );
RenderDevice*            g_RenderDevice( nullptr );
WorldRenderer*           g_WorldRenderer( nullptr );
GraphicsAssetManager*    g_GraphicsAssetManager( nullptr );
ShaderStageManager*      g_ShaderStageManager( nullptr );
DrawCommandBuilder*      g_DrawCommandBuilder( nullptr ); 
RenderableEntityManager* g_RenderableEntityManager( nullptr );
AudioDevice*             g_AudioDevice( nullptr );
FileSystemNative*        g_SaveFileSystem( nullptr );
FileSystemNative*        g_DataFileSystem( nullptr );
DynamicsWorld*           g_DynamicsWorld( nullptr );
Scene*                   g_CurrentScene( nullptr );

#if FLAN_DEVBUILD
FileSystemWatchdog*      g_FileSystemWatchdog( nullptr );
FileSystemNative*        g_DevFileSystem( nullptr );
TransactionHandler*      g_TransactionHandler( nullptr );
PhysicsDebugDraw*        g_PhysicsDebugDraw( nullptr );
#endif

