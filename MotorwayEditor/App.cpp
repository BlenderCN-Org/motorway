/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

#include <Network/GameClient.h>

#if FLAN_DEVBUILD
#include <Graphics/GraphicsProfiler.h>
#include <Core/Profiler.h>
#include "Core/FileSystem/FileSystemWatchdog.h"

#include <Physics/PhysicsDebugDraw.h>

#include <Rendering/CommandList.h>
#include <Rendering/CommandListPool.h>
#include <Display/DisplaySurfaceWin32.h>

#if FLAN_D3D11
#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_win32.h>

#include <Rendering/Direct3D11/CommandList.h>
#include <Rendering/Direct3D11/CommandListPool.h>
#include <Rendering/Direct3D11/RenderContext.h>
#include <imgui/examples/imgui_impl_dx11.h>
#elif FLAN_GL460
#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_win32.h>

#include <Rendering/OpenGL460/CommandList.h>
#include <Rendering/OpenGL460/CommandListPool.h>
#include <Rendering/OpenGL460/RenderContext.h>

#include <imgui/examples/imgui_impl_opengl3.h>
#elif FLAN_VULKAN
#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_win32.h>

#include <Rendering/Vulkan/CommandList.h>
#include <Rendering/Vulkan/CommandListPool.h>
#include <Rendering/Vulkan/RenderContext.h>

#include <imgui/examples/imgui_impl_vulkan.h>
#endif

#include <Framework/TransactionHandler/TransactionHandler.h>
#endif

#include <FileSystem/FileSystemObjectNative.h>
#include <Io/Scene.h>

#include <Framework/TransactionHandler/TransactionHandler.h>
#include <Framework/TransactionHandler/SceneNodeCopyCommand.h>
#include <Framework/TransactionHandler/SceneNodeDeleteCommand.h>

#include <Framework/SceneNodes/EmptySceneNode.h>
#include <Framework/Scene.h>
#include "EditorInterface.h"

static constexpr fnChar_t* const PROJECT_NAME = ( fnChar_t* const )FLAN_STRING( "MotorwayEditor" );

#define WIN_MODE_OPTION_LIST( option ) option( WINDOWED ) option( FULLSCREEN ) option( BORDERLESS )
FLAN_ENV_OPTION_LIST( WindowMode, WIN_MODE_OPTION_LIST )

FLAN_ENV_VAR( WindowWidth, "Defines application window width [0...]", 1280, int32_t )
FLAN_ENV_VAR( WindowHeight, "Defines application window height [0...]", 720, int32_t )
FLAN_ENV_VAR( WindowMode, "Defines application window mode [Windowed/Fullscreen/Borderless]", WINDOWED, eWindowMode )

FLAN_DEV_VAR( EnableCPUProfilerPrint, "Enables CPU Profiling Print on Screen [false/true]", false, bool )
FLAN_DEV_VAR( EnableCPUFPSPrint, "Enables CPU FPS Print on Screen [false/true]", true, bool )
FLAN_DEV_VAR( EnableDebugPhysicsColliders, "Enables Bullet's Debug Physics World Draw [false/true]", true, bool )
FLAN_ENV_VAR( CameraFOV, "Camera FieldOfView (in degrees)", 80.0f, float )
FLAN_ENV_VAR( MSAASamplerCount, "Defines MSAA sampler count [0/2/4/8]", 0, int32_t )
FLAN_ENV_VAR( EnableTemporalAA, "Enables Temporal Antialiasing [false/true]", false, bool )
FLAN_ENV_VAR( EnableFXAA, "Enables FXAA [false/true]", false, bool )

FLAN_DEV_VAR( CopiedNode, "Copied Node", nullptr, SceneNode* )
FLAN_DEV_VAR( PickedNode, "Picked Node", nullptr, SceneNode* )
FLAN_DEV_VAR( IsDevMenuVisible, "IsDevMenuVisible [false/true]", false, bool )

FLAN_DEV_VAR_PERSISTENT( EditorAutoSaveDelayInSeconds, "Auto save delay (in seconds)", 120.0f, float )


static CommandListPool* cmdListTest = nullptr;
static Timer           autoSaveTimer;

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
    g_CurrentScene.reset( new Scene() );

#if FLAN_DEVBUILD
    //g_GraphicsProfiler.reset( new GraphicsProfiler() );
    g_FileSystemWatchdog.reset( new FileSystemWatchdog() );
    g_TransactionHandler.reset( new TransactionHandler() );
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

            auto cameraNode = static_cast< FreeCameraSceneNode* >( g_CurrentScene->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) ) );
            cameraNode->enabled = true;

            auto mainCamera = cameraNode->camera;
            mainCamera->clearRenderPasses();
            mainCamera->addRenderPass( FLAN_STRING_HASH( "CascadedShadowMapCapture" ) );
            mainCamera->addRenderPass( FLAN_STRING_HASH( "AtmosphereRenderPass" ) );

            if ( MSAASamplerCount <= 1 ) {
                mainCamera->addRenderPass( FLAN_STRING_HASH( "WorldDepthPass" ) );
                mainCamera->addRenderPass( FLAN_STRING_HASH( "LightCullingPass" ) );
                mainCamera->addRenderPass( FLAN_STRING_HASH( "WorldLightPass" ) );
                mainCamera->addRenderPass( FLAN_STRING_HASH( "DebugWorldPass" ) );
                mainCamera->addRenderPass( FLAN_STRING_HASH( "LineRenderPass" ) );
            } else {
                mainCamera->addRenderPass( FLAN_STRING_HASH( "CopyTextureToMSAAPass" ) );
                mainCamera->addRenderPass( FLAN_STRING_HASH( "WorldDepthMSAAPass" ) );
                mainCamera->addRenderPass( FLAN_STRING_HASH( "LightCullingMSAAPass" ) );
                mainCamera->addRenderPass( FLAN_STRING_HASH( "WorldLightMSAAPass" ) );
                mainCamera->addRenderPass( FLAN_STRING_HASH( "DebugWorldMSAAPass" ) );
                mainCamera->addRenderPass( FLAN_STRING_HASH( "LineRenderPass" ) );

                mainCamera->addRenderPass( FLAN_STRING_HASH( std::string( "MSAADepthResolvePass" + std::to_string( MSAASamplerCount ) ).c_str() ) );

                if ( EnableTemporalAA ) {
                    mainCamera->addRenderPass( FLAN_STRING_HASH( std::string( "AntiAliasingPassMSAA" + std::to_string( MSAASamplerCount ) + "TAA" ).c_str() ) );
                } else {
                    mainCamera->addRenderPass( FLAN_STRING_HASH( std::string( "AntiAliasingPassMSAA" + std::to_string( MSAASamplerCount ) ).c_str() ) );
                }
            }

            mainCamera->addRenderPass( FLAN_STRING_HASH( "AutoExposurePass" ) );
            mainCamera->addRenderPass( FLAN_STRING_HASH( "BloomPass" ) );
            mainCamera->addRenderPass( FLAN_STRING_HASH( "CompositionPass" ) );

            if ( EnableFXAA ) {
                mainCamera->addRenderPass( FLAN_STRING_HASH( "FXAAPass" ) );
            }

            mainCamera->addRenderPass( FLAN_STRING_HASH( "HUDRenderPass" ) );
            mainCamera->addRenderPass( FLAN_STRING_HASH( "TextRenderPass" ) );
            mainCamera->addRenderPass( FLAN_STRING_HASH( "PresentPass" ) );
            g_CurrentScene->update( flan::framework::LOGIC_DELTA );

            accumulator -= flan::framework::LOGIC_DELTA;
        }
        g_Profiler.endSection();

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

        g_FileSystemWatchdog->OnFrame();

        if ( EnableDebugPhysicsColliders ) {
            g_PhysicsDebugDraw->onFrame();
        }

        //uint32_t winWidth, winHeight;
        //g_MainDisplaySurface->getSurfaceDimension( winWidth, winHeight );
        //g_DebugUI->onFrame( frameTime, g_DrawCommandBuilder.get(), winWidth, winHeight );
#endif

        g_Profiler.beginSection( "GameLogic::collectRenderKeys" );
            // Collect render keys from the current scene
            g_CurrentScene->collectRenderKeys( g_DrawCommandBuilder.get() );
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

#if FLAN_DEVBUILD
        auto cmdList = cmdListTest->allocateCmdList( g_RenderDevice.get() );
        DrawEditorInterface( interpolatedFrametime, cmdList );
#endif

        g_RenderDevice->present();
    }

#if FLAN_D3D11
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#elif FLAN_GL460
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif

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

    FLAN_CLOG << "Initialization done!" << std::endl;

#if FLAN_DEVBUILD
    //g_GraphicsProfiler->create( g_RenderDevice.get() );

    // TODO Remove this once menu and UI are set
#if FLAN_DEVBUILD
    g_InputMapper->pushContext( FLAN_STRING_HASH( "Editor" ) );

    // Free Camera
    g_InputMapper->addCallback( [&]( MappedInput& input, float frameTime ) {
        if ( !IsDevMenuVisible ) {
            auto cameraNode = ( FreeCameraSceneNode* )g_CurrentScene->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) );
            auto mainCamera = cameraNode->camera;

            // Camera Controls
            auto axisX = input.Ranges[FLAN_STRING_HASH( "CameraMoveHorizontal" )];
            auto axisY = input.Ranges[FLAN_STRING_HASH( "CameraMoveVertical" )];

            mainCamera->OnMouseUpdate( frameTime, axisX, axisY );

            if ( input.States.find( FLAN_STRING_HASH( "CameraMoveRight" ) ) != input.States.end() ) {
                mainCamera->MoveRight( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "CameraMoveLeft" ) ) != input.States.end() ) {
                mainCamera->MoveLeft( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "CameraMoveForward" ) ) != input.States.end() ) {
                mainCamera->MoveForward( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "CameraMoveBackward" ) ) != input.States.end() ) {
                mainCamera->MoveBackward( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "CameraLowerAltitude" ) ) != input.States.end() ) {
                mainCamera->LowerAltitude( frameTime );
            }

            if ( input.States.find( FLAN_STRING_HASH( "CameraTakeAltitude" ) ) != input.States.end() ) {
                mainCamera->TakeAltitude( frameTime );
            }
        }
    }, 0 );

    // Debug UI
    g_InputMapper->addCallback( [&]( MappedInput& input, float frameTime ) {
        // DebugUI
        if ( input.Actions.find( FLAN_STRING_HASH( "OpenDevMenu" ) ) != input.Actions.end() ) {
            IsDevMenuVisible = !IsDevMenuVisible;

            if ( IsDevMenuVisible ) {
                g_InputMapper->pushContext( FLAN_STRING_HASH( "DebugUI" ) );
            } else {
                g_InputMapper->popContext();
            }
        }

        auto cameraNode = ( FreeCameraSceneNode* )g_CurrentScene->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) );

        // Compatibility Hack; to remove later
        if ( cameraNode != nullptr ) {
            auto mainCamera = cameraNode->camera;

            if ( input.States.find( FLAN_STRING_HASH( "MoveCamera" ) ) != input.States.end() ) {
                auto axisX = input.Ranges[FLAN_STRING_HASH( "CameraMoveHorizontal" )];
                auto axisY = input.Ranges[FLAN_STRING_HASH( "CameraMoveVertical" )];

                mainCamera->OnMouseUpdate( frameTime, axisX, axisY );
            }
            ImGuiIO& io = ImGui::GetIO();

            if ( !io.WantCaptureMouse ) {
                if ( input.Actions.find( FLAN_STRING_HASH( "PickNode" ) ) != input.Actions.end() ) {
                    auto cameraData = mainCamera->GetData();
                    auto viewMat = glm::transpose( cameraData.viewMatrix );
                    auto projMat = cameraData.depthProjectionMatrix;

                    auto inverseViewProj = glm::inverse( projMat * viewMat );

                    glm::vec4 ray =
                    {
                        ( io.MousePos.x / io.DisplaySize.x ) * 2.f - 1.f,
                        ( 1.f - ( io.MousePos.y / io.DisplaySize.y ) ) * 2.f - 1.f,
                        0.0f,
                        1.0f
                    };

                    glm::vec4 rayOrigin =
                    {
                        ray.x * inverseViewProj[0][0] + ray.y * inverseViewProj[1][0] + ray.z * inverseViewProj[2][0] + ray.w * inverseViewProj[3][0],
                        ray.x * inverseViewProj[0][1] + ray.y * inverseViewProj[1][1] + ray.z * inverseViewProj[2][1] + ray.w * inverseViewProj[3][1],
                        ray.x * inverseViewProj[0][2] + ray.y * inverseViewProj[1][2] + ray.z * inverseViewProj[2][2] + ray.w * inverseViewProj[3][2],
                        ray.x * inverseViewProj[0][3] + ray.y * inverseViewProj[1][3] + ray.z * inverseViewProj[2][3] + ray.w * inverseViewProj[3][3],
                    };
                    rayOrigin *= ( 1.0f / rayOrigin.w );

                    ray.z = 1.0f;
                    glm::vec4 rayEnd =
                    {
                        ray.x * inverseViewProj[0][0] + ray.y * inverseViewProj[1][0] + ray.z * inverseViewProj[2][0] + ray.w * inverseViewProj[3][0],
                        ray.x * inverseViewProj[0][1] + ray.y * inverseViewProj[1][1] + ray.z * inverseViewProj[2][1] + ray.w * inverseViewProj[3][1],
                        ray.x * inverseViewProj[0][2] + ray.y * inverseViewProj[1][2] + ray.z * inverseViewProj[2][2] + ray.w * inverseViewProj[3][2],
                        ray.x * inverseViewProj[0][3] + ray.y * inverseViewProj[1][3] + ray.z * inverseViewProj[2][3] + ray.w * inverseViewProj[3][3],
                    };
                    rayEnd *= ( 1.0f / rayEnd.w );

                    auto rayDir = glm::normalize( rayEnd - rayOrigin );

                    glm::vec3 rayDirection = glm::vec3( rayDir );
                    glm::vec3 rayOrig = glm::vec3( rayOrigin );

                    Ray rayObj( rayOrig, rayDirection );
                    PickedNode = g_CurrentScene->intersect( rayObj );
                }
            }
        }

        // Default: Ctrl
        if ( input.States.find( FLAN_STRING_HASH( "Modifier1" ) ) != input.States.end() ) {
            if ( input.Actions.find( FLAN_STRING_HASH( "CopyNode" ) ) != input.Actions.end() ) {
                if ( PickedNode != nullptr ) {
                    CopiedNode = PickedNode;
                }
            }

            if ( input.Actions.find( FLAN_STRING_HASH( "OpenScene" ) ) != input.Actions.end() ) {
                fnString_t sceneName;
                if ( flan::core::DisplayFileOpenPrompt( sceneName, FLAN_STRING( "Asset Scene file (*.scene)\0*.scene" ), FLAN_STRING( "./" ), FLAN_STRING( "Save as a Scene asset" ) ) ) {
                    auto file = new FileSystemObjectNative( sceneName );
                    file->open( std::ios::binary | std::ios::in );

                    PickedNode = nullptr;
                    g_RenderableEntityManager->clear();

                    Scene* loadedScene = new Scene();

                    // Trigger scene change (flush CPU/GPU buffers; discard current game state; etc.)
                    g_CurrentScene.reset( loadedScene );

                    // Then load the scene
                    Io_ReadSceneFile( file, g_GraphicsAssetManager.get(), g_RenderableEntityManager.get(), *loadedScene );
                    file->close();
                    delete file;

                    for ( auto& node : g_CurrentScene->getSceneNodes() ) {
                        if ( node->rigidBody != nullptr ) {
                            g_DynamicsWorld->addRigidBody( node->rigidBody );
                        }
                    }

                    // Find active editor camera (dont use the gameplay one in editor mode!)
                    auto cameraNode = static_cast< FreeCameraSceneNode* >( loadedScene->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) ) );
                    if ( cameraNode != nullptr ) {
                        cameraNode->enabled = true;
                    } else {
                        FLAN_CWARN << "No camera defined/loaded; exepect garbage and weird stuff on screen..." << std::endl;
                    }

                    //    paString_t fileNameWithoutExtension, sceneNameWithExt;
                    //    pa::core::ExtractFilenameFromPath( sceneName, fileNameWithoutExtension );
                    //    pa::core::GetFilenameWithoutExtension( fileNameWithoutExtension, sceneNameWithExt );

                    //    g_RenderManager->LoadProbesFromDisk( g_GraphicsAssetManager.get(), paString_t( sceneNameWithExt.c_str() ) );

                    //    // Rebuild BVH
                }
            }

            if ( input.Actions.find( FLAN_STRING_HASH( "SaveScene" ) ) != input.Actions.end() ) {
                fnString_t sceneName;
                if ( flan::core::DisplayFileSavePrompt( sceneName, FLAN_STRING( "Asset Scene file (*.scene)\0*.scene" ), FLAN_STRING( "./" ), FLAN_STRING( "Save as a Scene asset" ) ) ) {
                    sceneName = fnString_t( sceneName.c_str() );
                    auto file = new FileSystemObjectNative( fnString_t( sceneName + FLAN_STRING( ".scene" ) ) );
                    file->open( std::ios::binary | std::ios::out );
                    Io_WriteSceneFile( g_CurrentScene.get(), file );
                    file->close();
                    delete file;
                }
            }

            if ( input.Actions.find( FLAN_STRING_HASH( "PasteNode" ) ) != input.Actions.end() ) {
                if ( CopiedNode != nullptr ) {
                    g_TransactionHandler->commit( new SceneNodeCopyCommand( PickedNode, g_CurrentScene.get(), g_RenderableEntityManager.get(), g_DynamicsWorld.get() ) );
                }
            }

            if ( input.Actions.find( FLAN_STRING_HASH( "Undo" ) ) != input.Actions.end() ) {
                g_TransactionHandler->undo();
            }

            if ( input.Actions.find( FLAN_STRING_HASH( "Redo" ) ) != input.Actions.end() ) {
                g_TransactionHandler->redo();
            }
        }

        if ( input.Actions.find( FLAN_STRING_HASH( "DeleteNode" ) ) != input.Actions.end() ) {
            if ( PickedNode != nullptr ) {
                g_TransactionHandler->commit( new SceneNodeDeleteCommand( PickedNode, g_CurrentScene.get(), g_RenderableEntityManager.get(), g_DynamicsWorld.get() ) );
                PickedNode = nullptr;
            }
        }
    }, -1 );

    autoSaveTimer.start();
#endif

    // Setup at least one base camera
    uint32_t surfaceWidth = 0, surfaceHeight = 0;
    g_MainDisplaySurface->getSurfaceDimension( surfaceWidth, surfaceHeight );

    auto testCam = new FreeCamera();
    testCam->SetProjectionMatrix( CameraFOV, static_cast<float>( surfaceWidth ), static_cast<float>( surfaceHeight ) );
    auto testCamNode = ( FreeCameraSceneNode* )g_CurrentScene->createFreeCamera( testCam, "DefaultCamera" );
    testCamNode->enabled = true;

    g_PhysicsDebugDraw->create( g_DynamicsWorld.get(), g_DrawCommandBuilder.get() );

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    cmdListTest = new CommandListPool();
    cmdListTest->create( g_RenderDevice.get(), 8 );

    ImGui_ImplWin32_Init( g_MainDisplaySurface->getNativeDisplaySurface()->Handle );

    // Setup style
    ImGui::StyleColorsDark();

    ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0.41f, 0.41f, 0.41f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_BorderShadow, ImVec4( 0.41f, 0.41f, 0.41f, 1.0f ) );

    ImGui::PushStyleColor( ImGuiCol_Separator, ImVec4( 0.14f, 0.14f, 0.14f, 1.0f ) );
    
    ImGui::PushStyleColor( ImGuiCol_Button,  ImVec4( 0.38f, 0.38f, 0.38f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.96f, 0.62f, 0.1f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.96f, 0.62f, 0.1f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_TitleBgActive, ImVec4( 0.96f, 0.62f, 0.1f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_SeparatorActive, ImVec4( 0.96f, 0.62f, 0.1f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_TextSelectedBg, ImVec4( 0.96f, 0.62f, 0.1f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_HeaderActive, ImVec4( 0.96f, 0.62f, 0.1f, 1.0f ) );
    
    ImGui::PushStyleColor( ImGuiCol_ScrollbarGrab, ImVec4( 0.96f, 0.62f, 0.1f, 1.0f ) );    
    ImGui::PushStyleColor( ImGuiCol_HeaderHovered, ImVec4( 0.27f, 0.31f, 0.35f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.27f, 0.31f, 0.35f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.27f, 0.31f, 0.35f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_Text,  ImVec4( 0.8f, 0.8f, 0.8f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_CheckMark, ImVec4( 0.1f, 0.1f, 0.1f, 1.0f ) );
    
    ImGui::PushStyleColor( ImGuiCol_TitleBg, ImVec4( 0.28f, 0.28f, 0.28f, 0.750f ) );
    ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.28f, 0.28f, 0.28f, 0.750f ) );
    ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.28f, 0.28f, 0.28f, 0.750f ) );
    ImGui::PushStyleColor( ImGuiCol_MenuBarBg, ImVec4( 0.28f, 0.28f, 0.28f, 0.750f ) );
    ImGui::PushStyleColor( ImGuiCol_Header, ImVec4( 0.28f, 0.28f, 0.28f, 0.750f ) );
    
    ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.28f, 0.28f, 0.28f, 0.750f ) );
    ImGui::PushStyleColor( ImGuiCol_PopupBg, ImVec4( 0.28f, 0.28f, 0.28f, 0.750f ) );

    ImGui::PushStyleColor( ImGuiCol_ScrollbarBg, ImVec4( 0.24f, 0.24f, 0.24f, 1.0f ) );

    g_Profiler.create();
    g_Profiler.drawOnScreen( EnableCPUProfilerPrint, 1.0f, 0.1f );
    g_FileSystemWatchdog->Create();
#endif

    return 0;
}