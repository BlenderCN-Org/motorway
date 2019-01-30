#include <Shared.h>
#include "Editor.h"

#include "EditorShared.h"
#include "Tickrate.h"

#include <Audio/AudioDevice.h>

#include <Core/Timer.h>
#include <Core/FileLogger.h>
#include <Core/Environment.h>

#include <Core/Allocators/LinearAllocator.h>
#include <Core/Allocators/GrowingStackAllocator.h>

#include <Display/DisplaySurface.h>

#include <Graphics/RenderPipeline.h>
#include <Graphics/ShaderCache.h>
#include <Graphics/WorldRenderer.h>
#include <Graphics/GraphicsAssetCache.h>
#include <Graphics/DrawCommandBuilder.h>

#include <Framework/Cameras/FreeCamera.h>
#include <Framework/Scene.h>

#include <Maths/Transform.h>

#include <FileSystem/VirtualFileSystem.h>
#include <FileSystem/FileSystemNative.h>

#include <Input/InputMapper.h>
#include <Input/InputReader.h>

#include <Rendering/RenderDevice.h>

#include <Core/EnvVarsRegister.h>
#include <Core/FramerateCounter.h>

#include "DefaultInputConfig.h"

#include <Graphics/LightGrid.h>

#include <thread>
#include <atomic>
#include <mutex>

static constexpr nyaChar_t* const PROJECT_NAME = ( nyaChar_t* const )NYA_STRING( "NyaEd" );

// CRT allocated memory for base heap allocation
char                    g_BaseBuffer[128];
void*                   g_AllocatedTable;
void*                   g_AllocatedVirtualMemory;

AudioDevice*            g_AudioDevice;
DisplaySurface*         g_DisplaySurface;
InputMapper*            g_InputMapper;
InputReader*            g_InputReader;
RenderDevice*           g_RenderDevice;
VirtualFileSystem*      g_VirtualFileSystem;
FileSystemNative*       g_SaveFileSystem;
FileSystemNative*       g_DataFileSystem;
FileSystemNative*       g_DevFileSystem;
ShaderCache*            g_ShaderCache;
WorldRenderer*          g_WorldRenderer;
GraphicsAssetCache*     g_GraphicsAssetCache;
DrawCommandBuilder*     g_DrawCommandBuilder;
LightGrid*              g_LightGrid;

Scene*                  g_SceneTest;
FreeCamera*             g_FreeCamera;

std::atomic_bool        g_ThreadSync( false );
std::mutex              g_SceneMutex;

FramerateCounter renderCounter = {};
FramerateCounter logicCounter = {};
FramerateCounter audioCounter = {};

// Game Specifics
#define WIN_MODE_OPTION_LIST( option ) option( WINDOWED ) option( FULLSCREEN ) option( BORDERLESS )
NYA_ENV_OPTION_LIST( WindowMode, WIN_MODE_OPTION_LIST )

NYA_ENV_VAR( WindowWidth, 1280, int32_t ) // "Defines application window width [0..N]"
NYA_ENV_VAR( WindowHeight, 720, int32_t ) // "Defines application window height [0..N]"
NYA_ENV_VAR( WindowMode, WINDOWED, eWindowMode ) // Defines application window mode [Windowed/Fullscreen/Borderless]

// Incomplete module declaration
class TextRenderingModule
{
public:
    void addOutlinedText( const char* text, float size, float x, float y, const glm::vec4& textColor = glm::vec4( 1, 1, 1, 1 ), const float outlineThickness = 0.80f );
};

void RegisterInputContexts()
{
    g_InputMapper->pushContext( NYA_STRING_HASH( "Editor" ) );

    // Free Camera
    g_InputMapper->addCallback( [&]( MappedInput& input, float frameTime ) {
        // Camera Controls
        auto axisX = input.Ranges[NYA_STRING_HASH( "CameraMoveHorizontal" )];
        auto axisY = input.Ranges[NYA_STRING_HASH( "CameraMoveVertical" )];

        g_FreeCamera->updateMouse( frameTime, axisX, axisY );

        if ( input.States.find( NYA_STRING_HASH( "CameraMoveRight" ) ) != input.States.end() ) {
            g_FreeCamera->moveRight( frameTime );
        }

        if ( input.States.find( NYA_STRING_HASH( "CameraMoveLeft" ) ) != input.States.end() ) {
            g_FreeCamera->moveLeft( frameTime );
        }

        if ( input.States.find( NYA_STRING_HASH( "CameraMoveForward" ) ) != input.States.end() ) {
            g_FreeCamera->moveForward( frameTime );
        }

        if ( input.States.find( NYA_STRING_HASH( "CameraMoveBackward" ) ) != input.States.end() ) {
            g_FreeCamera->moveBackward( frameTime );
        }

        if ( input.States.find( NYA_STRING_HASH( "CameraLowerAltitude" ) ) != input.States.end() ) {
            g_FreeCamera->lowerAltitude( frameTime );
        }

        if ( input.States.find( NYA_STRING_HASH( "CameraTakeAltitude" ) ) != input.States.end() ) {
            g_FreeCamera->takeAltitude( frameTime );
        }
    }, 0 );
}

void TestStuff()
{
    auto freeCameraId = g_SceneTest->FreeCameraDatabase.allocate();

    // Retrieve pointer to camera instance from scene db
    g_FreeCamera = &g_SceneTest->FreeCameraDatabase[freeCameraId];
    g_FreeCamera->setProjectionMatrix( 90.0f, static_cast<float>( WindowWidth ), static_cast<float>( WindowHeight ) );

    auto& meshTest = g_SceneTest->allocateStaticGeometry();
    auto& meshTransform = g_SceneTest->TransformDatabase[meshTest.transform];
    meshTransform.translate( glm::vec3( 32, 0, 0 ) );

    auto& geometry = g_SceneTest->RenderableMeshDatabase[meshTest.mesh];
    geometry.meshResource = g_GraphicsAssetCache->getMesh( NYA_STRING( "GameData/geometry/test.mesh" ) );

    PointLightData pointLightData;
    pointLightData.worldPosition = { 16, 0.5f, 0 };
    pointLightData.radius = 2.0f;
    pointLightData.lightPower = 2500.0f;
    pointLightData.colorRGB = { 1, 1, 1 };

    auto& pointLight = g_SceneTest->allocatePointLight();
    pointLight.pointLight = g_LightGrid->allocatePointLightData( std::forward<PointLightData>( pointLightData ) );
    auto& pointLightTransform = g_SceneTest->TransformDatabase[pointLight.transform];
    pointLightTransform.translate( pointLightData.worldPosition );
}

void Initialize()
{
    // Allocate memory for every subsystem
    g_AllocatedTable = nya::core::malloc( 1024 * 1024 * 1024 );
    g_AllocatedVirtualMemory = nya::core::PageAlloc( 256 * 1024 * 1024 );

    g_GlobalAllocator = new ( g_BaseBuffer ) LinearAllocator( 1024 * 1024 * 1024, g_AllocatedTable );
    g_GrowingGlobalAllocator = new ( g_BaseBuffer + sizeof( LinearAllocator ) ) GrowingStackAllocator( 256 * 1024 * 1024, g_AllocatedVirtualMemory, nya::core::GetPageSize() );

    NYA_CLOG << "Initializing '" << PROJECT_NAME << "'..." << std::endl;

    g_VirtualFileSystem = nya::core::allocate<VirtualFileSystem>( g_GlobalAllocator );

    nyaString_t cfgFilesDir;
    nya::core::RetrieveSavedGamesDirectory( cfgFilesDir );

    if ( cfgFilesDir.empty() ) {
        NYA_CWARN << "Failed to retrieve 'Saved Games' folder (this is expected behavior on Unix)" << std::endl;
        nya::core::RetrieveHomeDirectory( cfgFilesDir );

        NYA_ASSERT( !cfgFilesDir.empty(), "Failed to retrieve a suitable directory for savegame storage on your system..." );
    }

    // Prepare files/folders stored on the system fs
    // For now, configuration/save files will be stored in the same folder
    // This might get refactored later (e.g. to implement profile specific config/save for each system user)
    auto saveFolder = nya::core::allocate<FileSystemNative>( g_GlobalAllocator, nyaString_t( cfgFilesDir ) );

#if NYA_UNIX
    // Use *nix style configuration folder name
    nyaString_t configurationFolderName = NYA_STRING( "SaveData/.nyaed/" );
#else
    nyaString_t configurationFolderName = NYA_STRING( "SaveData/NyaEd/" );
#endif

    NYA_CLOG << "Mounting filesystems..." << std::endl;

    g_DataFileSystem = nya::core::allocate<FileSystemNative>( g_GlobalAllocator, NYA_STRING( "./data/" ) );

    g_VirtualFileSystem->mount( g_DataFileSystem, NYA_STRING( "GameData" ), 1 );

#if NYA_DEVBUILD
    NYA_CLOG << "Mounting devbuild filesystem..." << std::endl;

    g_DevFileSystem = nya::core::allocate<FileSystemNative>( g_GlobalAllocator, NYA_STRING( "./dev/" ) );
    g_VirtualFileSystem->mount( g_DevFileSystem, NYA_STRING( "GameData" ), 0 );
#endif

    auto SaveFolder = saveFolder->resolveFilename( NYA_STRING( "SaveData/" ), configurationFolderName );

    if ( !saveFolder->fileExists( SaveFolder ) ) {
        NYA_CLOG << "First run detected! Creating save folder at '" << SaveFolder << "'" << std::endl;

        saveFolder->createFolder( SaveFolder );
    }

    nya::core::free( g_GlobalAllocator, saveFolder );

    nya::core::OpenLogFile( SaveFolder, PROJECT_NAME );

    // Log generic stuff that could be useful
    NYA_COUT << PROJECT_NAME << " " << NYA_BUILD << "\n" << NYA_BUILD_DATE << "\n" << std::endl;

    NYA_CLOG << "SaveData folder at : '" << SaveFolder << "'" << std::endl;

    g_SaveFileSystem = nya::core::allocate<FileSystemNative>( g_GlobalAllocator, SaveFolder );
    g_VirtualFileSystem->mount( g_SaveFileSystem, NYA_STRING( "SaveData" ), UINT64_MAX );

    auto envConfigurationFile = g_VirtualFileSystem->openFile( NYA_STRING( "SaveData/environment.cfg" ), nya::core::eFileOpenMode::FILE_OPEN_MODE_READ );
    if ( envConfigurationFile == nullptr ) {
        NYA_CLOG << "Creating default user configuration!" << std::endl;
        auto newEnvConfigurationFile = g_VirtualFileSystem->openFile( NYA_STRING( "SaveData/environment.cfg" ), nya::core::eFileOpenMode::FILE_OPEN_MODE_WRITE );
        EnvironmentVariables::serialize( newEnvConfigurationFile );
        newEnvConfigurationFile->close();
    } else {
        NYA_CLOG << "Loading user configuration..." << std::endl;
        EnvironmentVariables::deserialize( envConfigurationFile );
        envConfigurationFile->close();
    }

    // Create and initialize subsystems
    g_DisplaySurface = nya::display::CreateDisplaySurface( g_GlobalAllocator, WindowWidth, WindowHeight );
    nya::display::SetCaption( g_DisplaySurface, PROJECT_NAME );

    g_InputMapper = nya::core::allocate<InputMapper>( g_GlobalAllocator );
    g_InputReader = nya::core::allocate<InputReader>( g_GlobalAllocator );
    g_InputReader->create();

    auto inputConfigurationFile = g_VirtualFileSystem->openFile( NYA_STRING( "SaveData/input.cfg" ), nya::core::eFileOpenMode::FILE_OPEN_MODE_READ );
    if ( inputConfigurationFile == nullptr ) {
        NYA_CLOG << "Creating default input configuration file..." << std::endl;

        auto newInputConfigurationFile = g_VirtualFileSystem->openFile( NYA_STRING( "SaveData/input.cfg" ), nya::core::eFileOpenMode::FILE_OPEN_MODE_WRITE );
        nya::core::WriteDefaultInputCfg( newInputConfigurationFile, g_InputReader->getActiveInputLayout() );
        newInputConfigurationFile->close();

        inputConfigurationFile = g_VirtualFileSystem->openFile( NYA_STRING( "SaveData/input.cfg" ), nya::core::eFileOpenMode::FILE_OPEN_MODE_READ );
    }

    NYA_CLOG << "Loading input configuration..." << std::endl;
    g_InputMapper->deserialize( inputConfigurationFile );
    inputConfigurationFile->close();

    g_AudioDevice = nya::core::allocate<AudioDevice>( g_GlobalAllocator, g_GlobalAllocator );
    g_AudioDevice->create();

    g_RenderDevice = nya::core::allocate<RenderDevice>( g_GlobalAllocator, g_GlobalAllocator );
    g_RenderDevice->create( g_DisplaySurface );

    g_ShaderCache = nya::core::allocate<ShaderCache>( g_GlobalAllocator, g_GlobalAllocator, g_RenderDevice, g_VirtualFileSystem );
    g_WorldRenderer = nya::core::allocate<WorldRenderer>( g_GlobalAllocator, g_GlobalAllocator );
    g_GraphicsAssetCache = nya::core::allocate<GraphicsAssetCache>( g_GlobalAllocator, g_GlobalAllocator, g_RenderDevice, g_ShaderCache, g_VirtualFileSystem );
    g_DrawCommandBuilder = nya::core::allocate<DrawCommandBuilder>( g_GlobalAllocator, g_GlobalAllocator );
    g_LightGrid = nya::core::allocate<LightGrid>( g_GlobalAllocator, g_GlobalAllocator );
    g_SceneTest = nya::core::allocate<Scene>( g_GlobalAllocator, g_GlobalAllocator );

    g_LightGrid->create( g_RenderDevice );
    g_WorldRenderer->loadCachedResources( g_RenderDevice, g_ShaderCache, g_GraphicsAssetCache );

    //g_RenderDevice->enableVerticalSynchronisation( true );

    RegisterInputContexts();

#if NYA_DEVBUILD
    TestStuff();
#endif
}

void CollectDrawCmds( const Scene::GameWorldState& snapshot, DrawCommandBuilder& drawCmdBuilder )
{
    for ( uint32_t staticGeomIdx = 0; staticGeomIdx < snapshot.StaticGeometryCount; staticGeomIdx++ ) {
        auto& geometry = snapshot.StaticGeometry[staticGeomIdx];

        auto& transform = snapshot.TransformDatabase[geometry.transform];
        auto& renderable = snapshot.RenderableMeshDatabase[geometry.mesh];

        // Check renderable flags (but don't cull the instance yet)
        if ( renderable.isVisible ) {
            drawCmdBuilder.addGeometryToRender( renderable.meshResource, transform.getWorldModelMatrix() );
        }
    }

    for ( uint32_t freeCameraIdx = 0; freeCameraIdx < snapshot.FreeCameraDatabase.usageIndex; freeCameraIdx++ ) {
        drawCmdBuilder.addCamera( &snapshot.FreeCameraDatabase[freeCameraIdx].getData() );
    }
}

void RenderLoop()
{
    thread_local Scene::GameWorldState gameWorldSnapshot = {};

    Timer updateTimer = {};
    float frameTime = static_cast<float>( nya::core::GetTimerDeltaAsSeconds( &updateTimer ) );

    while ( 1 ) {
        frameTime = static_cast< float >( nya::core::GetTimerDeltaAsSeconds( &updateTimer ) );

        if ( g_ThreadSync ) {
            break;
        }

        renderCounter.onFrame( frameTime );

        std::string fpsString = "Logic " + std::to_string( logicCounter.AvgDeltaTime ).substr( 0, 6 ) + " ms / " + std::to_string( logicCounter.MaxDeltaTime ).substr( 0, 6 ) + " ms\n"
            "Render " + std::to_string( renderCounter.AvgDeltaTime ).substr( 0, 6 ) + " ms (" + std::to_string( renderCounter.AvgFramePerSecond ).substr( 0, 6 ) + " FPS) / "
            + std::to_string( renderCounter.MaxDeltaTime ).substr( 0, 6 ) + " ms\n"
            "Audio " + std::to_string( audioCounter.AvgDeltaTime ).substr( 0, 6 ) + " ms / " + std::to_string( audioCounter.MaxDeltaTime ).substr( 0, 6 ) + " ms\n";

        g_WorldRenderer->textRenderModule->addOutlinedText( "Thread Profiling\n", 0.350f, 0.0f, 0.0f );
        g_WorldRenderer->textRenderModule->addOutlinedText( fpsString.c_str(), 0.350f, 0.0f, 15.0f, glm::vec4( 1, 1, 0, 1 ) );
        
        {
            std::unique_lock<std::mutex>( g_SceneMutex );
            g_SceneTest->getWorldStateSnapshot( gameWorldSnapshot );
        }

        CollectDrawCmds( gameWorldSnapshot, *g_DrawCommandBuilder );

        g_DrawCommandBuilder->buildRenderQueues( g_WorldRenderer );

        // Do pre-world render steps (update light grid, upload buffers, etc.)
        CommandList& cmdList = g_RenderDevice->allocateGraphicsCommandList();
        cmdList.begin();
            g_LightGrid->updateClusters( &cmdList );
        cmdList.end();
        g_RenderDevice->submitCommandList( &cmdList );

        g_WorldRenderer->drawWorld( g_RenderDevice, frameTime );

        g_RenderDevice->present();
    }
}

void AudioLoop()
{
    Timer updateTimer = {};

    float frameTime = static_cast<float>( nya::core::GetTimerDeltaAsSeconds( &updateTimer ) );

    while ( 1 ) {
        frameTime = static_cast<float>( nya::core::GetTimerDeltaAsSeconds( &updateTimer ) );

        if ( g_ThreadSync ) {
            break;
        }

        audioCounter.onFrame( frameTime );

        // IDEAS
        // - Play Queue (or smthing like that)
        // - Play one sound per audio frame
        // - Enqueue asynchronously via the logic loop
    }
}

void MainLoop()
{ 
    // Application main loop
    Timer updateTimer = {};

    float frameTime = static_cast<float>( nya::core::GetTimerDeltaAsSeconds( &updateTimer ) );
    double accumulator = 0.0;

    std::thread renderThread( RenderLoop );
    std::thread audioThread( AudioLoop );

    while ( 1 ) {
        nya::display::PollSystemEvents( g_DisplaySurface, g_InputReader );

        // Update run signal primitive
        g_ThreadSync.store( nya::display::HasReceivedQuitSignal( g_DisplaySurface ) );
        if ( g_ThreadSync ) {
            break;
        }

        frameTime = static_cast< float >( nya::core::GetTimerDeltaAsSeconds( &updateTimer ) );

        logicCounter.onFrame( frameTime );

        {
            std::unique_lock<std::mutex>( g_SceneMutex );
            accumulator += frameTime;
            while ( accumulator >= nya::editor::LOGIC_DELTA ) {
                // Update Input
                g_InputReader->onFrame( g_InputMapper );

                // Update Local Game Instance
                g_InputMapper->update( nya::editor::LOGIC_DELTA );
                g_InputMapper->clear();

                g_SceneTest->updateLogic( nya::editor::LOGIC_DELTA );
                accumulator -= nya::editor::LOGIC_DELTA;
            }
        }
    }

    // Finish render/audio thread
    renderThread.join();
    audioThread.join();
}

void Shutdown()
{
    g_LightGrid->destroy( g_RenderDevice );
    g_WorldRenderer->destroy( g_RenderDevice );
    g_GraphicsAssetCache->destroy();
    
    nya::core::free( g_GlobalAllocator, g_SceneTest );
    nya::core::free( g_GlobalAllocator, g_GraphicsAssetCache );
    nya::core::free( g_GlobalAllocator, g_WorldRenderer );
    nya::core::free( g_GlobalAllocator, g_ShaderCache );
    nya::core::free( g_GlobalAllocator, g_AudioDevice );
    nya::core::free( g_GlobalAllocator, g_DevFileSystem );
    nya::core::free( g_GlobalAllocator, g_DataFileSystem );
    nya::core::free( g_GlobalAllocator, g_SaveFileSystem );
    nya::core::free( g_GlobalAllocator, g_VirtualFileSystem );
    nya::core::free( g_GlobalAllocator, g_RenderDevice );
    nya::core::free( g_GlobalAllocator, g_InputReader );
    nya::core::free( g_GlobalAllocator, g_InputMapper );

    nya::display::DestroyDisplaySurface( g_DisplaySurface );

    g_GlobalAllocator->clear();
    g_GlobalAllocator->~LinearAllocator();

    nya::core::free( g_AllocatedTable );

    nya::core::CloseLogFile();
}

void nya::editor::Start()
{
    Initialize();   
    MainLoop();
    Shutdown();
}
