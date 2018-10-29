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
FLAN_ENV_VAR( CameraFOV, "Camera FieldOfView (in degrees)", 80.0f, floafot )
FLAN_ENV_VAR( MSAASamplerCount, "Defines MSAA sampler count [0/2/4/8]", 0, int32_t )
FLAN_ENV_VAR( EnableTemporalAA, "Enables Temporal Antialiasing [false/true]", false, bool )
FLAN_ENV_VAR( EnableFXAA, "Enables FXAA [false/true]", false, bool )

FLAN_DEV_VAR( CopiedNode, "Copied Node", nullptr, SceneNode* )
FLAN_DEV_VAR( PickedNode, "Picked Node", nullptr, SceneNode* )
FLAN_DEV_VAR( IsDevMenuVisible, "IsDevMenuVisible [false/true]", false, bool )

FLAN_DEV_VAR_PERSISTENT( EditorAutoSaveDelayInSeconds, "Auto save delay (in seconds)", 120.0f, float )


#include <Core/Allocators/HeapAllocator.h>
#include <Core/Allocators/PoolAllocator.h>

#include <Core/Allocators/LinearAllocator.h>

// CRT allocated memory for base heap allocation
char                g_BaseBuffer[1024];
Timer               g_EditorAutoSaveTimer;
CommandListPool*    g_EditorCmdListPool = nullptr;

Heap* g_HeapTest;

void CreateSubsystems()
{
    g_HeapTest = new ( g_BaseBuffer ) Heap( 1024 * 1024 * 1024 );

    // Create global instances whenever the Application ctor is called
    g_FileLogger = g_HeapTest->allocate<FileLogger>( PROJECT_NAME );
    g_VirtualFileSystem = g_HeapTest->allocate<VirtualFileSystem>();
    g_TaskManager = g_HeapTest->allocate<TaskManager>();
    g_MainDisplaySurface = g_HeapTest->allocate<DisplaySurface>( PROJECT_NAME );

    g_InputReader = ( g_HeapTest->allocate<InputReader>() );
    g_InputMapper = ( g_HeapTest->allocate<InputMapper>() );
    g_RenderDevice = ( g_HeapTest->allocate<RenderDevice>() );
    g_WorldRenderer = ( g_HeapTest->allocate<WorldRenderer>() );
    g_ShaderStageManager = ( g_HeapTest->allocate<ShaderStageManager>( g_RenderDevice, g_VirtualFileSystem ) );
    g_GraphicsAssetManager = ( g_HeapTest->allocate<GraphicsAssetManager>( g_RenderDevice, g_ShaderStageManager, g_VirtualFileSystem, g_HeapTest ) );
    g_DrawCommandBuilder = ( g_HeapTest->allocate<DrawCommandBuilder>() );
    g_RenderableEntityManager = ( g_HeapTest->allocate<RenderableEntityManager>() );
    g_AudioDevice = ( g_HeapTest->allocate<AudioDevice>() );
    g_DynamicsWorld = ( g_HeapTest->allocate<DynamicsWorld>() );
    g_CurrentScene = ( g_HeapTest->allocate<Scene>() );

#if FLAN_DEVBUILD
    //g_GraphicsProfiler =( new GraphicsProfiler() );
    g_FileSystemWatchdog = ( g_HeapTest->allocate<FileSystemWatchdog>() );
    g_TransactionHandler = ( g_HeapTest->allocate<TransactionHandler>() );
    g_PhysicsDebugDraw = g_HeapTest->allocate<PhysicsDebugDraw>();
#endif
}

void Shutdown()
{
#if FLAN_D3D11
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#elif FLAN_GL460
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif

#if FLAN_DEVBUILD
    g_VirtualFileSystem->unmount( g_DevFileSystem );
#endif

    g_WorldRenderer->destroy();
    g_GraphicsAssetManager->destroy();

    g_VirtualFileSystem->unmount( g_SaveFileSystem );
    g_VirtualFileSystem->unmount( g_DataFileSystem );

    g_FileLogger->close();
}

int InitializeSubsystems()
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
    auto saveFolder = g_HeapTest->allocate<FileSystemNative>( fnString_t( cfgFilesDir ) );

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

    g_HeapTest->free( saveFolder );

    FLAN_CLOG << "SaveData folder at : '" << aloneSaveFolder << "'" << std::endl;
    FLAN_CLOG << "Mounting filesystems..." << std::endl;

    g_SaveFileSystem = ( g_HeapTest->allocate<FileSystemNative>( aloneSaveFolder ) );
    g_DataFileSystem = ( g_HeapTest->allocate<FileSystemNative>( FLAN_STRING( "./data/" ) ) );

    g_VirtualFileSystem->mount( g_SaveFileSystem, FLAN_STRING( "SaveData" ), UINT64_MAX );
    g_VirtualFileSystem->mount( g_DataFileSystem, FLAN_STRING( "GameData" ), 1 );

#if FLAN_DEVBUILD
    FLAN_CLOG << "Mounting devbuild filesystem..." << std::endl;

    g_DevFileSystem = ( g_HeapTest->allocate<FileSystemNative>( FLAN_STRING( "./dev/" ) ) );
    g_VirtualFileSystem->mount( g_DevFileSystem, FLAN_STRING( "GameData" ), 0 );
#endif

    // Open FileLogger (setup console output stream redirection)
    g_FileLogger->open( aloneSaveFolder );

    // Log generic stuff that could be useful
    FLAN_COUT << PROJECT_NAME << " " << FLAN_BUILD << std::endl
        << FLAN_BUILD_DATE << "\n" << std::endl;

    g_TaskManager->create( g_HeapTest );

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
    g_RenderDevice->create( g_MainDisplaySurface );

    FLAN_IMPORT_VAR_PTR( EnableVSync, bool );
    g_RenderDevice->setVSyncState( *EnableVSync );

    g_WorldRenderer->create( g_RenderDevice );
    g_WorldRenderer->loadCachedResources( g_ShaderStageManager, g_GraphicsAssetManager );

    g_DrawCommandBuilder->create( g_TaskManager, g_RenderableEntityManager, g_GraphicsAssetManager, g_WorldRenderer );
    g_RenderableEntityManager->create( g_RenderDevice );

    FLAN_CLOG << "Initializing audio subsystems..." << std::endl;
    g_AudioDevice->create();

    FLAN_CLOG << "Initializing physics subsystems..." << std::endl;
    g_DynamicsWorld->create();

    FLAN_CLOG << "Initialization done!" << std::endl;

#if FLAN_DEVBUILD
    //g_GraphicsProfiler->create( g_RenderDevice );

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
                    auto file = g_HeapTest->allocate<FileSystemObjectNative>( sceneName );
                    file->open( std::ios::binary | std::ios::in );

                    PickedNode = nullptr;
                    g_RenderableEntityManager->clear();

                    Scene* loadedScene = g_HeapTest->allocate<Scene>();

                    // Trigger scene change (flush CPU/GPU buffers; discard current game state; etc.)
                    g_CurrentScene = loadedScene;

                    // Then load the scene
                    Io_ReadSceneFile( file, g_GraphicsAssetManager, g_RenderableEntityManager, *loadedScene );
                    file->close();
                    g_HeapTest->free( file );

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

                    //    g_RenderManager->LoadProbesFromDisk( g_GraphicsAssetManager, paString_t( sceneNameWithExt.c_str() ) );

                    //    // Rebuild BVH
                }
            }

            if ( input.Actions.find( FLAN_STRING_HASH( "SaveScene" ) ) != input.Actions.end() ) {
                fnString_t sceneName;
                if ( flan::core::DisplayFileSavePrompt( sceneName, FLAN_STRING( "Asset Scene file (*.scene)\0*.scene" ), FLAN_STRING( "./" ), FLAN_STRING( "Save as a Scene asset" ) ) ) {
                    sceneName = fnString_t( sceneName.c_str() );
                    auto file = new FileSystemObjectNative( fnString_t( sceneName + FLAN_STRING( ".scene" ) ) );
                    file->open( std::ios::binary | std::ios::out );
                    Io_WriteSceneFile( g_CurrentScene, file );
                    file->close();
                    delete file;
                }
            }

            if ( input.Actions.find( FLAN_STRING_HASH( "PasteNode" ) ) != input.Actions.end() ) {
                if ( CopiedNode != nullptr ) {
                    g_TransactionHandler->commit( new SceneNodeCopyCommand( PickedNode, g_CurrentScene, g_RenderableEntityManager, g_DynamicsWorld ) );
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
                g_TransactionHandler->commit( new SceneNodeDeleteCommand( PickedNode, g_CurrentScene, g_RenderableEntityManager, g_DynamicsWorld ) );
                PickedNode = nullptr;
            }
        }
    }, -1 );

    g_EditorAutoSaveTimer.start();
#endif

    // Setup at least one base camera
    uint32_t surfaceWidth = 0, surfaceHeight = 0;
    g_MainDisplaySurface->getSurfaceDimension( surfaceWidth, surfaceHeight );

    auto testCam = new FreeCamera();
    testCam->SetProjectionMatrix( CameraFOV, static_cast<float>( surfaceWidth ), static_cast<float>( surfaceHeight ) );
    auto testCamNode = ( FreeCameraSceneNode* )g_CurrentScene->createFreeCamera( testCam, "DefaultCamera" );
    testCamNode->enabled = true;

    g_PhysicsDebugDraw->create( g_DynamicsWorld, g_DrawCommandBuilder );

    DirectionalLightData sunLight = {};
    sunLight.isSunLight = true;
    sunLight.intensityInLux = 100000.0f;
    sunLight.angularRadius = 0.007f;
    const float solidAngle = ( 2.0f * glm::pi<float>() ) * ( 1.0f - cos( sunLight.angularRadius ) );

    sunLight.illuminanceInLux = sunLight.intensityInLux * solidAngle;
    sunLight.sphericalCoordinates = glm::vec2( 1.0f, 0.5f );
    sunLight.direction = flan::core::SphericalToCarthesianCoordinates( sunLight.sphericalCoordinates.x, sunLight.sphericalCoordinates.y );
    auto sunLightEntity = g_RenderableEntityManager->createDirectionalLight( std::move( sunLight ) );
    auto sunLightNode = g_CurrentScene->createDirectionalLight( sunLightEntity );

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // ImGui draw cmdlist
    g_EditorCmdListPool = new CommandListPool();
    g_EditorCmdListPool->create( g_RenderDevice, 2 );

    ImGui_ImplWin32_Init( g_MainDisplaySurface->getNativeDisplaySurface()->Handle );

    // Setup style
    ImGui::StyleColorsDark();

    ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0.41f, 0.41f, 0.41f, 1.0f ) );
    ImGui::PushStyleColor( ImGuiCol_BorderShadow, ImVec4( 0.41f, 0.41f, 0.41f, 1.0f ) );

    ImGui::PushStyleColor( ImGuiCol_Separator, ImVec4( 0.14f, 0.14f, 0.14f, 1.0f ) );

    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.38f, 0.38f, 0.38f, 1.0f ) );
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
    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.8f, 0.8f, 0.8f, 1.0f ) );
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

void RebuildCameraPipeline( Camera* mainCamera )
{
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

    FLAN_IMPORT_VAR_PTR( SSAAMultiplicator, float )
        if ( *SSAAMultiplicator > 1.0f ) {
            mainCamera->addRenderPass( FLAN_STRING_HASH( "SSAAResolvePass" ) );
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
}

int motorway::game::Start()
{
    CreateSubsystems();

    const int initializationResult = InitializeSubsystems();
    if ( initializationResult != 0 ) {
        FLAN_CERR << "Motorway initialization failed! (error code: " << initializationResult << ")" << std::endl;
        return 1;
    }

    // Application main loop
    Timer updateTimer = {};
    float frameTime = static_cast<float>( updateTimer.getDeltaAsSeconds() );

    float previousTime = frameTime;
    double accumulator = 0.0;
    FramerateCounter logicCounter = {};
    while ( 1 ) {
        FLAN_PROFILE_SECTION( g_MainDisplaySurface->pumpEvents( g_InputReader ) );
        if ( g_MainDisplaySurface->shouldQuit() ) {
            break;
        }

        frameTime = static_cast<float>( updateTimer.getDeltaAsSeconds() );

        logicCounter.onFrame( frameTime );
        g_Profiler.drawOnScreen( EnableCPUProfilerPrint, 0.30f, 0.1f );
        g_Profiler.onFrame( g_WorldRenderer );

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
            g_InputReader->onFrame( g_InputMapper );

            // Update Local Game Instance
            g_InputMapper->update( flan::framework::LOGIC_DELTA );
            g_InputMapper->clear();

            g_DynamicsWorld->update( flan::framework::LOGIC_DELTA );

            auto cameraNode = static_cast< FreeCameraSceneNode* >( g_CurrentScene->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) ) );
            cameraNode->enabled = true;

            RebuildCameraPipeline( cameraNode->camera );

            g_CurrentScene->update( flan::framework::LOGIC_DELTA );

            accumulator -= flan::framework::LOGIC_DELTA;
        }
        g_Profiler.endSection();

        if ( EnableCPUFPSPrint ) {
            std::string fpsString = "CPU: " + std::to_string( logicCounter.AvgDeltaTime ).substr( 0, 6 ) + " ms (" + std::to_string( logicCounter.AvgFramePerSecond ).substr( 0, 6 ) + " FPS) / "
                + std::to_string( logicCounter.MinDeltaTime ).substr( 0, 6 ) + " ms / "
                + std::to_string( logicCounter.MaxDeltaTime ).substr( 0, 6 ) + " ms\n";

            g_WorldRenderer->drawDebugText( fpsString, 0.3f, 1.0f, 0.0f, 0.50f, glm::vec4( 1.0f, 1.0f, 0.0f, 1.00f ) );
        }

        g_FileSystemWatchdog->OnFrame();

        if ( EnableDebugPhysicsColliders ) {
            g_PhysicsDebugDraw->onFrame();
        }

        // Collect render keys from the current scene
        FLAN_PROFILE_SECTION( g_CurrentScene->collectRenderKeys( g_DrawCommandBuilder ) );

        // Prepare drawcalls and pipelines for the GPU (don't setup anything yet)
        FLAN_PROFILE_SECTION( g_DrawCommandBuilder->buildCommands( g_RenderDevice, g_WorldRenderer ) );

        float interpolatedFrametime = static_cast< float >( accumulator ) / flan::framework::LOGIC_DELTA;
        FLAN_PROFILE_SECTION( g_WorldRenderer->onFrame( interpolatedFrametime, g_TaskManager ) );

#if FLAN_DEVBUILD
        // Print debug memory usage
        float globalHeapMib = ( float )g_GlobalHeapUsage / ( 1024.0f * 1024.0f );
        std::string globalHeapUsage = "Global Heap Usage: " + std::to_string( globalHeapMib ).substr( 0, 6 ) + "MiB";
        g_WorldRenderer->drawDebugText( globalHeapUsage, 0.3f, 0.0f, 0.0f );

        float heapMib = ( float )g_HeapTest->getMemoryUsage() / ( 1024.0f * 1024.0f );
        std::string heapUsage = "Application Heap Usage: " + std::to_string( heapMib ).substr( 0, 6 ) + "/1024.0MiB (" + std::to_string( g_HeapTest->getAllocationCount() ) + " allocations)";

        g_WorldRenderer->drawDebugText( heapUsage, 0.3f, 0.0f, 0.07f );

        CommandList* cmdList = g_EditorCmdListPool->allocateCmdList( g_RenderDevice );
        DrawEditorInterface( interpolatedFrametime, cmdList );
#endif

        g_RenderDevice->present();
    }

    Shutdown();

    return 0;
}
