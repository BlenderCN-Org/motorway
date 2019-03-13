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
#include <Graphics/LightGrid.h>
#include <Graphics/DrawCommandBuilder.h>

#include <Framework/Cameras/FreeCamera.h>
#include <Framework/Scene.h>
#include <Framework/Mesh.h>

#include <Maths/Helpers.h>
#include <Maths/Transform.h>
#include <Maths/Vector.h>
#include <Maths/Quaternion.h>
#include <Maths/CoordinatesSystems.h>
#include <Maths/MatrixTransformations.h>

#include <FileSystem/VirtualFileSystem.h>
#include <FileSystem/FileSystemNative.h>

#include <Input/InputMapper.h>
#include <Input/InputReader.h>

#include <Rendering/RenderDevice.h>

#include <Core/EnvVarsRegister.h>
#include <Core/FramerateCounter.h>

#include "DefaultInputConfig.h"

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

// Game Specifics
#define WIN_MODE_OPTION_LIST( option ) option( WINDOWED ) option( FULLSCREEN ) option( BORDERLESS )
NYA_ENV_OPTION_LIST( WindowMode, WIN_MODE_OPTION_LIST )

NYA_ENV_VAR( ScreenSize, nyaVec2u( 1280, 720 ), nyaVec2u ) // "Defines application screen size [0..N]"
NYA_ENV_VAR( WindowMode, WINDOWED, eWindowMode ) // Defines application window mode [Windowed/Fullscreen/Borderless]
NYA_ENV_VAR( CameraFOV, 80.0f, float ) // "Camera FieldOfView (in degrees)"
NYA_ENV_VAR( ImageQuality, 1.0f, float ) // "Image Quality Scale (in degrees) [0.1..N]"
NYA_ENV_VAR( EnableVSync, false, bool ) // "Enable Vertical Synchronisation [false/true]"
NYA_ENV_VAR( EnableTAA, false, bool ) // "Enable TemporalAntiAliasing [false/true]"
NYA_ENV_VAR( MSAASamplerCount, 1, uint32_t ) // "MultiSampledAntiAliasing Sampler Count [1..8]"

// Incomplete module declaration
class TextRenderingModule
{
public:
    void addOutlinedText( const char* text, float size, float x, float y, const nyaVec4f& textColor = nyaVec4f( 1, 1, 1, 1 ), const float outlineThickness = 0.80f );
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
    NYA_CLOG << "Initializing stuff..." << std::endl;

    auto freeCameraId = g_SceneTest->FreeCameraDatabase.allocate();

    // Retrieve pointer to camera instance from scene db
    g_FreeCamera = &g_SceneTest->FreeCameraDatabase[freeCameraId];
    g_FreeCamera->setProjectionMatrix( CameraFOV, static_cast<float>( ScreenSize.x ), static_cast<float>( ScreenSize.y ) );
    
    g_FreeCamera->setImageQuality( ImageQuality );
    g_FreeCamera->setMSAASamplerCount( MSAASamplerCount );

    // Toggle camera flags based on user settings
    auto& cameraFlags = g_FreeCamera->getUpdatableFlagset();
    cameraFlags.enableTAA = EnableTAA;

    auto& meshTest = g_SceneTest->allocateStaticGeometry();

    auto& geometry = g_SceneTest->RenderableMeshDatabase[meshTest.mesh];
    geometry.meshResource = g_GraphicsAssetCache->getMesh( NYA_STRING( "GameData/geometry/test.mesh" ) );

    auto& geometryTransform = g_SceneTest->TransformDatabase[meshTest.transform];

    auto& planeTest = g_SceneTest->allocateStaticGeometry();

    auto& geometryPlane = g_SceneTest->RenderableMeshDatabase[planeTest.mesh];
    geometryPlane.meshResource = g_GraphicsAssetCache->getMesh( NYA_STRING( "GameData/geometry/plane.mesh" ) );

    for ( int j = 0; j < 5; j++ ) {
        for ( int i = 0; i < 6; i++ ) {
            PointLightData pointLightData;
            pointLightData.worldPosition = { static_cast< float >( i ), 0.25f, static_cast< float>( j ) };
            pointLightData.radius = 0.5f;
            pointLightData.lightPower = 100.0f;
            pointLightData.colorRGB = { static_cast< float >( rand() ) / RAND_MAX, static_cast< float >( rand() ) / RAND_MAX, static_cast< float >( rand() ) / RAND_MAX };

            auto& pointLight = g_SceneTest->allocatePointLight();
            pointLight.pointLight = g_LightGrid->allocatePointLightData( std::forward<PointLightData>( pointLightData ) );

            auto& pointLightTransform = g_SceneTest->TransformDatabase[pointLight.transform];
            pointLightTransform.setWorldTranslation( pointLightData.worldPosition );
        }
    }

    DirectionalLightData sunLight = {};
    sunLight.isSunLight = true;
    sunLight.intensityInLux = 100000.0f;
    sunLight.angularRadius = 0.00935f / 2.0f;
    const float solidAngle = ( 2.0f * nya::maths::PI<float>() ) * ( 1.0f - cos( sunLight.angularRadius ) );

    sunLight.illuminanceInLux = sunLight.intensityInLux * solidAngle;
    sunLight.sphericalCoordinates = nyaVec2f( 0.50f, 0.5f );
    sunLight.direction = nya::maths::SphericalToCarthesianCoordinates( sunLight.sphericalCoordinates.x, sunLight.sphericalCoordinates.y );

    auto& dirLight = g_SceneTest->allocateDirectionalLight();
    dirLight.directionalLight = g_LightGrid->updateDirectionalLightData( std::forward<DirectionalLightData>( sunLight ) );

    IBLProbeData globalProbe = {};
    globalProbe.worldPosition = { 0, 6, 0 };
    globalProbe.isFallbackProbe = true;

    auto& globalProbeNode = g_SceneTest->allocateIBLProbe();
    globalProbeNode.iblProbe = g_LightGrid->updateGlobalIBLProbeData( std::forward<IBLProbeData>( globalProbe ) );

    IBLProbeData localProbe = {};
    localProbe.worldPosition = { 4, 4, 0 };
    localProbe.radius = 14.0f;
    localProbe.isFallbackProbe = false;

    nyaMat4x4f probeModelMatrix = nya::maths::MakeTranslationMat( localProbe.worldPosition ) * nya::maths::MakeScaleMat( localProbe.radius );
    localProbe.inverseModelMatrix = probeModelMatrix.inverse();

    auto& localProbeNode = g_SceneTest->allocateIBLProbe();
    localProbeNode.iblProbe = g_LightGrid->allocateLocalIBLProbeData( std::forward<IBLProbeData>( localProbe ) );

    const AABB& aabbMesh = geometry.meshResource->getMeshAABB();
    g_LightGrid->setSceneBounds( aabbMesh.maxPoint, nyaVec3f( -16.0f, 0.0f, -16.0f ) );
}

void InitializeIOSubsystems()
{
    NYA_CLOG << "Initializing I/O subsystems..." << std::endl;

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
}

void InitializeInputSubsystems()
{
    NYA_CLOG << "Initializing input subsystems..." << std::endl;

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
}

void InitializeRenderSubsystems()
{
    NYA_CLOG << "Initializing render subsystems..." << std::endl;

    // Create and initialize subsystems
    g_DisplaySurface = nya::display::CreateDisplaySurface( g_GlobalAllocator, ScreenSize.x, ScreenSize.y );
    nya::display::SetCaption( g_DisplaySurface, PROJECT_NAME );

    g_RenderDevice = nya::core::allocate<RenderDevice>( g_GlobalAllocator, g_GlobalAllocator );
    g_RenderDevice->create( g_DisplaySurface );

    g_ShaderCache = nya::core::allocate<ShaderCache>( g_GlobalAllocator, g_GlobalAllocator, g_RenderDevice, g_VirtualFileSystem );
    g_WorldRenderer = nya::core::allocate<WorldRenderer>( g_GlobalAllocator, g_GlobalAllocator );
    g_GraphicsAssetCache = nya::core::allocate<GraphicsAssetCache>( g_GlobalAllocator, g_GlobalAllocator, g_RenderDevice, g_ShaderCache, g_VirtualFileSystem );
    g_DrawCommandBuilder = nya::core::allocate<DrawCommandBuilder>( g_GlobalAllocator, g_GlobalAllocator );
    g_LightGrid = nya::core::allocate<LightGrid>( g_GlobalAllocator, g_GlobalAllocator );

    g_LightGrid->loadCachedResources( g_RenderDevice, g_ShaderCache, g_GraphicsAssetCache );
    g_WorldRenderer->loadCachedResources( g_RenderDevice, g_ShaderCache, g_GraphicsAssetCache );

    g_RenderDevice->enableVerticalSynchronisation( EnableVSync );
}

void InitializeAudioSubsystems()
{
    NYA_CLOG << "Initializing audio subsystems..." << std::endl;

    g_AudioDevice = nya::core::allocate<AudioDevice>( g_GlobalAllocator, g_GlobalAllocator );
    g_AudioDevice->create();
}

void InitializeGameLogicSubsystems()
{
    NYA_CLOG << "Initializing game logic subsystems..." << std::endl;

    g_SceneTest = nya::core::allocate<Scene>( g_GlobalAllocator, g_GlobalAllocator );
}

void InitializeMemorySubsystems()
{
    NYA_CLOG << "Initializing memory subsystems..." << std::endl;

    // Allocate memory for every subsystem
    g_AllocatedTable = nya::core::malloc( 1024 * 1024 * 1024 );
    g_AllocatedVirtualMemory = nya::core::PageAlloc( 256 * 1024 * 1024 );

    g_GlobalAllocator = new ( g_BaseBuffer ) LinearAllocator( 1024 * 1024 * 1024, g_AllocatedTable );
    g_GrowingGlobalAllocator = new ( g_BaseBuffer + sizeof( LinearAllocator ) ) GrowingStackAllocator( 256 * 1024 * 1024, g_AllocatedVirtualMemory, nya::core::GetPageSize() );
}

void Initialize()
{
    InitializeMemorySubsystems();
    InitializeIOSubsystems();

    NYA_COUT << PROJECT_NAME << " " << NYA_BUILD << "\n" << NYA_BUILD_DATE << "\nCompiled with: " << NYA_COMPILER << "\n" << std::endl;

    InitializeInputSubsystems();
    InitializeRenderSubsystems();
    InitializeAudioSubsystems();
    InitializeGameLogicSubsystems();

    RegisterInputContexts();

#if NYA_DEVBUILD
    TestStuff();
#endif
}

void MainLoop()
{
    // Application main loop
    Timer updateTimer = {};
    FramerateCounter logicCounter = {};

    float frameTime = static_cast<float>( nya::core::GetTimerDeltaAsSeconds( &updateTimer ) );
    double accumulator = 0.0;

    while ( 1 ) {
        g_Profiler.onFrame();

        nya::display::PollSystemEvents( g_DisplaySurface, g_InputReader );

        if ( nya::display::HasReceivedQuitSignal( g_DisplaySurface ) ) {
            break;
        }

        frameTime = static_cast< float >( nya::core::GetTimerDeltaAsSeconds( &updateTimer ) );

        logicCounter.onFrame( frameTime );

        accumulator += frameTime;
        
        NYA_BEGIN_PROFILE_SCOPE( "Fixed-step updates" )
            while ( accumulator >= nya::editor::LOGIC_DELTA ) {
                // Update Input
                g_InputReader->onFrame( g_InputMapper );

                // Update Local Game Instance
                g_InputMapper->update( nya::editor::LOGIC_DELTA );
                g_InputMapper->clear();

                g_SceneTest->updateLogic( nya::editor::LOGIC_DELTA );
                accumulator -= nya::editor::LOGIC_DELTA;
            }
        NYA_END_PROFILE_SCOPE()

        NYA_BEGIN_PROFILE_SCOPE( "Rendering" )
            std::string fpsString = "Main Loop " + std::to_string( logicCounter.AvgDeltaTime ).substr( 0, 6 ) + " ms / " + std::to_string( logicCounter.MaxDeltaTime ).substr( 0, 6 ) + " ms (" + std::to_string( logicCounter.AvgFramePerSecond ).substr( 0, 6 ) + " FPS)";
            
            g_WorldRenderer->TextRenderModule->addOutlinedText( "Thread Profiling", 0.350f, 0.0f, 0.0f );
            g_WorldRenderer->TextRenderModule->addOutlinedText( fpsString.c_str(), 0.350f, 0.0f, 15.0f, nyaVec4f( 1, 1, 0, 1 ) );
       
            const std::string& profileString = g_Profiler.getProfilingSummaryString();
            g_WorldRenderer->TextRenderModule->addOutlinedText( profileString.c_str(), 0.350f, 256.0f, 0.0f );

            g_SceneTest->collectDrawCmds( *g_DrawCommandBuilder );
            g_DrawCommandBuilder->buildRenderQueues( g_WorldRenderer, g_LightGrid );

            g_WorldRenderer->drawWorld( g_RenderDevice, frameTime );
        NYA_END_PROFILE_SCOPE()

        g_RenderDevice->present();
    }
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
