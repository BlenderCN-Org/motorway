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
#include "App.h"

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

#include <Core/Environment.h>
#include <Core/Timer.h>
#include <Core/ScopedTimer.h>
#include <Core/FramerateCounter.h>

#include <Audio/AudioDevice.h>

#include <Framework/Tickrate.h>
#include <Physics/DynamicsWorld.h>

#include "Core/DefaultInputConfig.h"
#include "Game/GameLogic.h"

#include <Network/GameClient.h>

#if FLAN_DEVBUILD
#include <Graphics/GraphicsProfiler.h>
#include <Core/Profiler.h>
#include <Physics/PhysicsDebugDraw.h>
#endif

static constexpr fnChar_t* const PROJECT_NAME = ( fnChar_t* const )FLAN_STRING( "MotorwayClient" );

#define WIN_MODE_OPTION_LIST( option ) option( WINDOWED ) option( FULLSCREEN ) option( BORDERLESS )
FLAN_ENV_OPTION_LIST( WindowMode, WIN_MODE_OPTION_LIST )

FLAN_ENV_VAR( WindowWidth, "Defines application window width [0...]", 1280, int32_t )
FLAN_ENV_VAR( WindowHeight, "Defines application window height [0...]", 720, int32_t )
FLAN_ENV_VAR( WindowMode, "Defines application window mode [Windowed/Fullscreen/Borderless]", WINDOWED, eWindowMode )

FLAN_DEV_VAR( EnableCPUProfilerPrint, "Enables CPU Profiling Print on Screen [false/true]", false, bool )
FLAN_DEV_VAR( EnableCPUFPSPrint, "Enables CPU FPS Print on Screen [false/true]", true, bool )
FLAN_DEV_VAR( EnableDebugPhysicsColliders, "Enables Bullet's Debug Physics World Draw [false/true]", true, bool )

App::App()
{
    // Create global instances whenever the Application ctor is called
    g_FileLogger.reset( new FileLogger( PROJECT_NAME ) );
    g_VirtualFileSystem.reset( new VirtualFileSystem() );
    g_TaskManager.reset( new TaskManager() );
    g_MainDisplaySurface.reset( new DisplaySurface( PROJECT_NAME ) );
    g_InputReader.reset( new InputReader() );
    g_InputMapper.reset( new InputMapper() );
    g_RenderDevice.reset( new RenderDevice() );
    g_WorldRenderer.reset( new WorldRenderer() );
    g_ShaderStageManager.reset( new ShaderStageManager( g_RenderDevice.get(), g_VirtualFileSystem.get() ) );
    g_GraphicsAssetManager.reset( new GraphicsAssetManager( g_RenderDevice.get(), g_ShaderStageManager.get(), g_VirtualFileSystem.get() ) );
    g_DrawCommandBuilder.reset( new DrawCommandBuilder() );
    g_RenderableEntityManager.reset( new RenderableEntityManager() );
    g_AudioDevice.reset( new AudioDevice() );
    g_DynamicsWorld.reset( new DynamicsWorld() );
    g_GameClient.reset( new GameClient() );

    g_GameLogic.reset( new GameLogic() );

#if FLAN_DEVBUILD
    g_PhysicsDebugDraw.reset( new PhysicsDebugDraw() );
#endif
}

App::~App()
{

}

int App::launch()
{
    if ( initialize() != 0 ) {
        return 1;
    }

    // Application main loop
    Timer updateTimer = {};

    float frameTime = static_cast<float>( updateTimer.getDeltaAsSeconds() );

    float previousTime = frameTime;
    double accumulator = 0.0;
    FramerateCounter logicCounter = {};

    while ( 1 ) {
        g_Profiler.beginSection( "DisplaySurface::pumpEvents" );
            g_MainDisplaySurface->pumpEvents( g_InputReader.get() );
        g_Profiler.endSection();

        if ( g_MainDisplaySurface->shouldQuit() ) {
            break;
        }

        frameTime = static_cast<float>( updateTimer.getDeltaAsSeconds() );

#if FLAN_DEVBUILD
        logicCounter.onFrame( frameTime );

        //g_Profiler.beginSection( "GraphicsProfiler::onFrame" );
        //g_GraphicsProfiler->onFrame( g_RenderDevice.get(), g_WorldRenderer.get() );
        //g_Profiler.endSection();

        g_Profiler.drawOnScreen( EnableCPUProfilerPrint, 0.30f, 0.1f );
        g_Profiler.onFrame( g_WorldRenderer.get() );
#endif

        // Avoid spiral of death
        if ( frameTime > 0.2500f ) {
            frameTime = 0.2500f;
        }

        previousTime += frameTime;

        // Do fixed step updates and interpolate between each App state
        accumulator += frameTime;

        g_Profiler.beginSection( "Fixed Updates" );
        while ( accumulator >= flan::framework::LOGIC_DELTA ) {
            // Update Input
            g_InputReader->onFrame( g_InputMapper.get() );

            // Update Local Game Instance
            g_InputMapper->update( flan::framework::LOGIC_DELTA );
            g_InputMapper->clear();

            g_DynamicsWorld->update( flan::framework::LOGIC_DELTA );

            g_GameLogic->update( flan::framework::LOGIC_DELTA );

            accumulator -= flan::framework::LOGIC_DELTA;
        }
        g_Profiler.endSection();

        g_GameClient->OnFrame();

#if FLAN_DEVBUILD
        //// Compute GPU average delta time
        //auto gpuTime = g_GraphicsProfiler->getSectionResultArray();

        //double gpuTimeAvg = 0.0;
        //int sampleCount = 0;
        //for ( int i = 0; i < GraphicsProfiler::TOTAL_QUERY_COUNT; ) {
        //    if ( gpuTime[i] == -1.0 ) {
        //        i += GraphicsProfiler::MAX_PROFILE_SECTION_COUNT;
        //        continue;
        //    }

        //    gpuTimeAvg += gpuTime[i];
        //    sampleCount++;
        //    i++;
        //}

        //gpuTimeAvg /= sampleCount;

        if ( EnableCPUFPSPrint ) {
            std::string fpsString = "CPU: " + std::to_string( logicCounter.AvgDeltaTime ).substr( 0, 6 ) + " ms (" + std::to_string( logicCounter.AvgFramePerSecond ).substr( 0, 6 ) + " FPS) / "
                + std::to_string( logicCounter.MinDeltaTime ).substr( 0, 6 ) + " ms / "
                + std::to_string( logicCounter.MaxDeltaTime ).substr( 0, 6 ) + " ms\n"; // GPU: " + std::to_string( gpuTimeAvg ).substr( 0, 6 ) + " ms;

            g_WorldRenderer->drawDebugText( fpsString, 0.3f, 1.0f, 0.0f, 0.50f, glm::vec4( 1.0f, 1.0f, 0.0f, 1.00f ) );
        }

        if ( EnableDebugPhysicsColliders ) {
            g_PhysicsDebugDraw->onFrame();
        }

        //uint32_t winWidth, winHeight;
        //g_MainDisplaySurface->getSurfaceDimension( winWidth, winHeight );
        //g_DebugUI->onFrame( frameTime, g_DrawCommandBuilder.get(), winWidth, winHeight );
#endif

        g_Profiler.beginSection( "GameLogic::collectRenderKeys" );
            // Collect render keys from the current scene
            g_GameLogic->collectRenderKeys( g_DrawCommandBuilder.get() );
        g_Profiler.endSection();

        // Prepare drawcalls and pipelines for the GPU (don't setup anything yet)
        g_Profiler.beginSection( "DrawCommandBuilder::buildCommands" );
            g_DrawCommandBuilder->buildCommands( g_RenderDevice.get(), g_WorldRenderer.get() );
        g_Profiler.endSection();

        float interpolatedFrametime = static_cast< float >( accumulator ) / flan::framework::LOGIC_DELTA;

        //g_GraphicsProfiler->beginSection( g_RenderDevice.get(), "GPU" );
        g_Profiler.beginSection( "WorldRenderer::onFrame" );
            g_WorldRenderer->onFrame( interpolatedFrametime, g_TaskManager.get() );
        g_Profiler.endSection();
        //g_GraphicsProfiler->endSection( g_RenderDevice.get() );

        g_RenderDevice->present();
    }

    g_WorldRenderer->destroy();
    g_GraphicsAssetManager->destroy();

    return 0;
}

int App::initialize()
{
    const ScopedTimer AppInitializationTimer( FLAN_STRING( "AppInitializationTimer" ) );

    FLAN_CLOG << "Initializing '" << PROJECT_NAME << "'..." << std::endl;

    fnString_t cfgFilesDir;
    flan::core::RetrieveSavedGamesDirectory( cfgFilesDir );

    if ( cfgFilesDir.empty() ) {
        FLAN_CWARN << "Failed to retrieve 'Saved Games' folder (this is expected behavior on Unix)" << std::endl;
        flan::core::RetrieveHomeDirectory( cfgFilesDir );

        if ( cfgFilesDir.empty() ) {
            FLAN_CERR << "Failed to retrieve a suitable directory for savegame storage on your system..." << std::endl;
            FLAN_CERR << "The Application will now close" << std::endl;
            return 1;
        }
    }

    // Prepare files/folders stored on the system fs
    // For now, configuration/save files will be stored in the same folder
    // This might get refactored later (e.g. to implement profile specific config/save for each system user)
    auto saveFolder = new FileSystemNative( fnString_t( cfgFilesDir ) );

#if FLAN_UNIX
    // Use *nix style configuration folder name
    fnString_t configurationFolderName = FLAN_STRING( "SaveData/.motorway/" );
#else
    fnString_t configurationFolderName = FLAN_STRING( "SaveData/Motorway/" );
#endif

    auto aloneSaveFolder = saveFolder->resolveFilename( FLAN_STRING( "SaveData/" ), configurationFolderName );

    if ( !saveFolder->fileExists( aloneSaveFolder ) ) {
        FLAN_CLOG << "First run detected! Creating save folder at '" << aloneSaveFolder << "'" << std::endl;

        saveFolder->createFolder( aloneSaveFolder );
    }

    delete saveFolder;

    FLAN_CLOG << "SaveData folder at : '" << aloneSaveFolder << "'" << std::endl;
    FLAN_CLOG << "Mounting filesystems..." << std::endl;
    
    g_SaveFileSystem.reset( new FileSystemNative( aloneSaveFolder ) );
    g_DataFileSystem.reset( new FileSystemNative( FLAN_STRING( "./data/" ) ) );

    g_VirtualFileSystem->mount( g_SaveFileSystem.get(), FLAN_STRING( "SaveData" ), UINT64_MAX );
    g_VirtualFileSystem->mount( g_DataFileSystem.get(), FLAN_STRING( "GameData" ), 1 );

#if FLAN_DEVBUILD
    FLAN_CLOG << "Mounting devbuild filesystem..." << std::endl;

    g_DevFileSystem.reset( new FileSystemNative( FLAN_STRING( "./dev/" ) ) );
    g_VirtualFileSystem->mount( g_DevFileSystem.get(), FLAN_STRING( "GameData" ), 0 );
#endif

    // Open FileLogger (setup console output stream redirection)
    g_FileLogger->open( aloneSaveFolder );

    // Log generic stuff that could be useful
    FLAN_COUT << PROJECT_NAME << " " << FLAN_BUILD << std::endl
        << FLAN_BUILD_DATE << "\n" << std::endl;

    g_TaskManager->create();

    // Parse Environment Configuration
    FLAN_IMPORT_VAR_PTR( RebuildGameCfgFile, bool )

    auto envConfigurationFile = g_VirtualFileSystem->openFile( FLAN_STRING( "SaveData/Game.cfg" ), flan::core::eFileOpenMode::FILE_OPEN_MODE_READ );
    if ( envConfigurationFile == nullptr || *RebuildGameCfgFile ) {
        FLAN_CLOG << "Creating default user configuration!" << std::endl;
        auto newEnvConfigurationFile = g_VirtualFileSystem->openFile( FLAN_STRING( "SaveData/Game.cfg" ), flan::core::eFileOpenMode::FILE_OPEN_MODE_WRITE );
        EnvironmentVariables::serialize( newEnvConfigurationFile );
        newEnvConfigurationFile->close();
    } else {
        FLAN_CLOG << "Loading user configuration..." << std::endl;
        EnvironmentVariables::deserialize( envConfigurationFile );
        envConfigurationFile->close();
    }

    g_MainDisplaySurface->create( WindowWidth, WindowHeight );

    g_InputReader->create();

    // Parse Input Configuration
    FLAN_IMPORT_VAR_PTR( RebuildInputCfgFile, bool )

    auto inputConfigurationFile = g_VirtualFileSystem->openFile( FLAN_STRING( "SaveData/Input.cfg" ), flan::core::eFileOpenMode::FILE_OPEN_MODE_READ );
    if ( inputConfigurationFile == nullptr || *RebuildInputCfgFile ) {
        FLAN_CLOG << "Creating default input configuration file..." << std::endl;

        auto newInputConfigurationFile = g_VirtualFileSystem->openFile( FLAN_STRING( "SaveData/Input.cfg" ), flan::core::eFileOpenMode::FILE_OPEN_MODE_WRITE );
        alone::core::WriteDefaultInputCfg( newInputConfigurationFile, g_InputReader->getActiveInputLayout() );
        newInputConfigurationFile->close();

        inputConfigurationFile = g_VirtualFileSystem->openFile( FLAN_STRING( "SaveData/Input.cfg" ), flan::core::eFileOpenMode::FILE_OPEN_MODE_READ );
    }

    FLAN_CLOG << "Loading input configuration..." << std::endl;
    g_InputMapper->deserialize( inputConfigurationFile );
    inputConfigurationFile->close();

    // Rendering/Graphics systems
    FLAN_CLOG << "Initializing rendering subsystems..." << std::endl;
    g_RenderDevice->create( g_MainDisplaySurface.get() );

    FLAN_IMPORT_VAR_PTR( EnableVSync, bool );
    g_RenderDevice->setVSyncState( *EnableVSync );

    g_WorldRenderer->create( g_RenderDevice.get() );
    g_WorldRenderer->loadCachedResources( g_ShaderStageManager.get(), g_GraphicsAssetManager.get() );

    g_DrawCommandBuilder->create( g_TaskManager.get(), g_RenderableEntityManager.get(), g_GraphicsAssetManager.get(), g_WorldRenderer.get() );
    g_RenderableEntityManager->create( g_RenderDevice.get() );

    FLAN_CLOG << "Initializing audio subsystems..." << std::endl;
    g_AudioDevice->create();

    FLAN_CLOG << "Initializing physics subsystems..." << std::endl;
    g_DynamicsWorld->create();

    g_GameClient->ConnectTo( "127.0.0.1", 4583 );

    // Project specific stuff
    g_GameLogic->create();

    FLAN_CLOG << "Initialization done!" << std::endl;

#if FLAN_DEVBUILD
    g_PhysicsDebugDraw->create( g_DynamicsWorld.get(), g_DrawCommandBuilder.get() );

    g_Profiler.create();
    g_Profiler.drawOnScreen( EnableCPUProfilerPrint, 1.0f, 0.1f );
#endif

    return 0;
}
