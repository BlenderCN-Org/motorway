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
    along with this program.  If not, see <https://www.gnu.org/licenses/*.
*/
#pragma once

class FileLogger;
class VirtualFileSystem;
class TaskManager;
class DisplaySurface;
class InputReader;
class InputMapper;
class RenderDevice;
class WorldRenderer;
class GraphicsAssetManager;
class ShaderStageManager;
class DrawCommandBuilder;
class RenderableEntityManager;
class AudioDevice;
class FileSystemNative;
class DynamicsWorld;
class Scene;
class CommandListPool;

#if FLAN_DEVBUILD
class GraphicsProfiler;
class FileSystemWatchdog;
class TransactionHandler;
class PhysicsDebugDraw;
#endif

extern FileLogger*              g_FileLogger;
extern VirtualFileSystem*       g_VirtualFileSystem;
extern TaskManager*             g_TaskManager;
extern DisplaySurface*          g_MainDisplaySurface;
extern InputReader*             g_InputReader;
extern InputMapper*             g_InputMapper;
extern RenderDevice*            g_RenderDevice;
extern WorldRenderer*           g_WorldRenderer;
extern GraphicsAssetManager*    g_GraphicsAssetManager;
extern ShaderStageManager*      g_ShaderStageManager;
extern DrawCommandBuilder*      g_DrawCommandBuilder;
extern RenderableEntityManager* g_RenderableEntityManager;
extern AudioDevice*             g_AudioDevice;
extern DynamicsWorld*           g_DynamicsWorld;

extern FileSystemNative*        g_SaveFileSystem;
extern FileSystemNative*        g_DataFileSystem;

extern Scene*                   g_CurrentScene;
extern CommandListPool*         g_EditorCmdListPool;

#if FLAN_DEVBUILD
extern FileSystemWatchdog*      g_FileSystemWatchdog;
extern FileSystemNative*        g_DevFileSystem;
extern TransactionHandler*      g_TransactionHandler;
extern PhysicsDebugDraw*        g_PhysicsDebugDraw;
#endif
