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

#include "Game/GameLogic.h"

#if FLAN_DEVBUILD
#include <Graphics/GraphicsProfiler.h>
#include "Core/FileSystem/FileSystemWatchdog.h"
#include <Framework/TransactionHandler/TransactionHandler.h>
#include <Physics/PhysicsDebugDraw.h>
#endif

std::unique_ptr<FileLogger>              g_FileLogger( nullptr );
std::unique_ptr<VirtualFileSystem>       g_VirtualFileSystem( nullptr );
std::unique_ptr<TaskManager>             g_TaskManager( nullptr );
std::unique_ptr<DisplaySurface>          g_MainDisplaySurface( nullptr );
std::unique_ptr<InputReader>             g_InputReader( nullptr );
std::unique_ptr<InputMapper>             g_InputMapper( nullptr );
std::unique_ptr<RenderDevice>            g_RenderDevice( nullptr );
std::unique_ptr<WorldRenderer>           g_WorldRenderer( nullptr );
std::unique_ptr<GraphicsAssetManager>    g_GraphicsAssetManager( nullptr );
std::unique_ptr<ShaderStageManager>      g_ShaderStageManager( nullptr );
std::unique_ptr<GameLogic>               g_GameLogic( nullptr );
std::unique_ptr<DrawCommandBuilder>      g_DrawCommandBuilder( nullptr ); 
std::unique_ptr<RenderableEntityManager> g_RenderableEntityManager( nullptr );
std::unique_ptr<AudioDevice>             g_AudioDevice( nullptr );
std::unique_ptr<FileSystemNative>        g_SaveFileSystem( nullptr );
std::unique_ptr<FileSystemNative>        g_DataFileSystem( nullptr );
std::unique_ptr<DynamicsWorld>           g_DynamicsWorld( nullptr );
std::unique_ptr<GameClient>              g_GameClient( nullptr );

#if FLAN_DEVBUILD
std::unique_ptr<GraphicsProfiler>        g_GraphicsProfiler( nullptr );
std::unique_ptr<FileSystemWatchdog>      g_FileSystemWatchdog( nullptr );
std::unique_ptr<FileSystemNative>        g_DevFileSystem( nullptr );
std::unique_ptr<TransactionHandler>      g_TransactionHandler( nullptr );
std::unique_ptr<PhysicsDebugDraw>        g_PhysicsDebugDraw( nullptr );
#endif

