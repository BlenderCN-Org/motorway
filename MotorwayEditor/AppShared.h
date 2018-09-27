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

#if FLAN_DEVBUILD
class GraphicsProfiler;
class FileSystemWatchdog;
class TransactionHandler;
class PhysicsDebugDraw;
#endif

extern std::unique_ptr<FileLogger>              g_FileLogger;
extern std::unique_ptr<VirtualFileSystem>       g_VirtualFileSystem;
extern std::unique_ptr<TaskManager>             g_TaskManager;
extern std::unique_ptr<DisplaySurface>          g_MainDisplaySurface;
extern std::unique_ptr<InputReader>             g_InputReader;
extern std::unique_ptr<InputMapper>             g_InputMapper;
extern std::unique_ptr<RenderDevice>            g_RenderDevice;
extern std::unique_ptr<WorldRenderer>           g_WorldRenderer;
extern std::unique_ptr<GraphicsAssetManager>    g_GraphicsAssetManager;
extern std::unique_ptr<ShaderStageManager>      g_ShaderStageManager;
extern std::unique_ptr<DrawCommandBuilder>      g_DrawCommandBuilder;
extern std::unique_ptr<RenderableEntityManager> g_RenderableEntityManager;
extern std::unique_ptr<AudioDevice>             g_AudioDevice;
extern std::unique_ptr<DynamicsWorld>           g_DynamicsWorld;

extern std::unique_ptr<FileSystemNative>        g_SaveFileSystem;
extern std::unique_ptr<FileSystemNative>        g_DataFileSystem;

extern std::unique_ptr<Scene>                   g_CurrentScene;

#if FLAN_DEVBUILD
extern std::unique_ptr<GraphicsProfiler>        g_GraphicsProfiler;
extern std::unique_ptr<FileSystemWatchdog>      g_FileSystemWatchdog;
extern std::unique_ptr<FileSystemNative>        g_DevFileSystem;
extern std::unique_ptr<TransactionHandler>      g_TransactionHandler;
extern std::unique_ptr<PhysicsDebugDraw>        g_PhysicsDebugDraw;
#endif
