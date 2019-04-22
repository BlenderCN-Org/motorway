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

#include <Graphics/RenderModules/TextRenderingModule.h>
#include <Graphics/RenderModules/LineRenderingModule.h>

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

#include <Framework/GUI/Screen.h>
#include <Framework/GUI/Panel.h>
#include <Framework/GUI/Label.h>
#include <Framework/GUI/Button.h>

#include "DefaultInputConfig.h"

static constexpr const nyaChar_t* const PROJECT_NAME = static_cast<const nyaChar_t* const>( NYA_STRING( "NyaEd" ) );

// CRT allocated memory for base heap allocation
static char                    g_BaseBuffer[128];
static void*                   g_AllocatedTable;
static void*                   g_AllocatedVirtualMemory;

static AudioDevice*            g_AudioDevice;
static DisplaySurface*         g_DisplaySurface;
static InputMapper*            g_InputMapper;
static InputReader*            g_InputReader;
static RenderDevice*           g_RenderDevice;
static VirtualFileSystem*      g_VirtualFileSystem;
static FileSystemNative*       g_SaveFileSystem;
static FileSystemNative*       g_DataFileSystem;
static FileSystemNative*       g_DevFileSystem;
static ShaderCache*            g_ShaderCache;
static WorldRenderer*          g_WorldRenderer;
static GraphicsAssetCache*     g_GraphicsAssetCache;
static DrawCommandBuilder*     g_DrawCommandBuilder;
static LightGrid*              g_LightGrid;

static Scene*                  g_SceneTest;
static FreeCamera*             g_FreeCamera;
static GUIScreen*              g_DebugGUI;
static GUILabel*               g_FramerateGUILabel;
static bool                    g_IsDevMenuVisible = false;

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

void RegisterInputContexts()
{
    g_InputMapper->pushContext( NYA_STRING_HASH( "Editor" ) );

    // Free Camera
    g_InputMapper->addCallback( [&]( MappedInput& input, float frameTime ) {
        if ( g_IsDevMenuVisible ) {
            return;
        }

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

    // DebugUI
    g_InputMapper->addCallback( [&]( MappedInput& input, float frameTime ) {
        if ( input.Actions.find( NYA_STRING_HASH( "OpenDevMenu" ) ) != input.Actions.end() ) {
            g_IsDevMenuVisible = !g_IsDevMenuVisible;

            if ( g_IsDevMenuVisible ) {
                g_InputMapper->pushContext( NYA_STRING_HASH( "DebugUI" ) );
            } else {
                g_InputMapper->popContext();
            }
        }

        auto rawX = nya::maths::clamp( static_cast<float>( g_InputReader->getAbsoluteAxisValue( nya::input::eInputAxis::MOUSE_X ) ), 0.0f, static_cast<float>( ScreenSize.x ) );
        auto rawY = nya::maths::clamp( static_cast<float>( g_InputReader->getAbsoluteAxisValue( nya::input::eInputAxis::MOUSE_Y ) ), 0.0f, static_cast<float>( ScreenSize.y ) );

        g_DebugGUI->onMouseCoordinatesUpdate( rawX, rawY );
        if ( input.States.find( NYA_STRING_HASH( "MouseClick" ) ) != input.States.end() ) {
            g_DebugGUI->onLeftMouseButtonDown( rawX, rawY );
        } else {
            g_DebugGUI->onLeftMouseButtonUp();
        }
    }, -1 );
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
    
    auto& planeTest = g_SceneTest->allocateStaticGeometry();

    auto& geometryPlane = g_SceneTest->RenderableMeshDatabase[planeTest.mesh];
    geometryPlane.meshResource = g_GraphicsAssetCache->getMesh( NYA_STRING( "GameData/geometry/plane.mesh" ) );
    geometryPlane.renderDepth = 0;

    for ( int j = 0; j < 5; j++ ) {
        for ( int i = 0; i < 6; i++ ) {
            PointLightData pointLightData;
            pointLightData.worldPosition = { static_cast< float >( i ), 0.25f, static_cast< float>( j ) };
            pointLightData.radius = 0.50f;
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
    const float solidAngle = ( 2.0f * nya::maths::PI<float>() ) * static_cast<float>( 1.0f - cosf( sunLight.angularRadius ) );

    sunLight.illuminanceInLux = sunLight.intensityInLux * solidAngle;
    sunLight.sphericalCoordinates = nyaVec2f( 0.50f, 0.5f );
    sunLight.direction = nya::maths::SphericalToCarthesianCoordinates( sunLight.sphericalCoordinates.x, sunLight.sphericalCoordinates.y );

    auto& dirLight = g_SceneTest->allocateDirectionalLight();
    dirLight.directionalLight = g_LightGrid->updateDirectionalLightData( std::forward<DirectionalLightData>( sunLight ) );

    IBLProbeData globalProbe = {};
    globalProbe.worldPosition = { 4, 8, 4 };
    globalProbe.isFallbackProbe = true;

    auto& globalProbeNode = g_SceneTest->allocateIBLProbe();
    globalProbeNode.iblProbe = g_LightGrid->updateGlobalIBLProbeData( std::forward<IBLProbeData>( globalProbe ) );
    {
        IBLProbeData localProbe = {};
        localProbe.worldPosition = { 0, 10, 2 };
        localProbe.radius = 4.0f;
        localProbe.isFallbackProbe = false;

        nyaMat4x4f probeModelMatrix = nya::maths::MakeTranslationMat( localProbe.worldPosition ) * nya::maths::MakeScaleMat( localProbe.radius );
        localProbe.inverseModelMatrix = probeModelMatrix.inverse();

        auto& localProbeNode = g_SceneTest->allocateIBLProbe();
        localProbeNode.iblProbe = g_LightGrid->allocateLocalIBLProbeData( std::forward<IBLProbeData>( localProbe ) );
    }
    {
        IBLProbeData localProbe = {};
        localProbe.worldPosition = { -10, 10, 2 };
        localProbe.radius = 4.0f;
        localProbe.isFallbackProbe = false;

        nyaMat4x4f probeModelMatrix = nya::maths::MakeTranslationMat( localProbe.worldPosition ) * nya::maths::MakeScaleMat( localProbe.radius );
        localProbe.inverseModelMatrix = probeModelMatrix.inverse();

        auto& localProbeNode = g_SceneTest->allocateIBLProbe();
        localProbeNode.iblProbe = g_LightGrid->allocateLocalIBLProbeData( std::forward<IBLProbeData>( localProbe ) );
    }
    {
        IBLProbeData localProbe = {};
        localProbe.worldPosition = { -10, 10, 12 };
        localProbe.radius = 4.0f;
        localProbe.isFallbackProbe = false;

        nyaMat4x4f probeModelMatrix = nya::maths::MakeTranslationMat( localProbe.worldPosition ) * nya::maths::MakeScaleMat( localProbe.radius );
        localProbe.inverseModelMatrix = probeModelMatrix.inverse();

        auto& localProbeNode = g_SceneTest->allocateIBLProbe();
        localProbeNode.iblProbe = g_LightGrid->allocateLocalIBLProbeData( std::forward<IBLProbeData>( localProbe ) );
    }
    const AABB& aabbMesh = geometry.meshResource->getMeshAABB();
    g_LightGrid->setSceneBounds( nyaVec3f( 20, 20, 20 ), nyaVec3f( -20, -20, -20 ) );

    g_DebugGUI = nya::core::allocate<GUIScreen>( g_GlobalAllocator, g_GlobalAllocator ); 
    g_DebugGUI->setVirtualScreenSize( nyaVec2u( 1280u, 720u ) );

    GUIPanel& panelTest = g_DebugGUI->allocatePanel();
    panelTest.VirtualPosition = nyaVec2f( 640.0f, 480.0f );
    panelTest.VirtualSize = nyaVec2f( 250.0f, 120.0f );
    panelTest.IsDraggable = true;
    panelTest.PanelMaterial = g_GraphicsAssetCache->getMaterial( NYA_STRING( "GameData/materials/HUD/DefaultMaterial.mat" ) );
   
    g_FramerateGUILabel = g_DebugGUI->allocateWidget<GUILabel>();
    g_FramerateGUILabel->VirtualPosition = nyaVec2f( 995.0f, 0.0f );
    g_FramerateGUILabel->VirtualSize.x = 0.40f;
    g_FramerateGUILabel->ColorAndAlpha = nyaVec4f( 0.9f, 0.9f, 0.0f, 1.0f );
  
    GUILabel* windowLabelTest = g_DebugGUI->allocateWidget<GUILabel>();
    windowLabelTest->VirtualPosition = nyaVec2f( 0.01f, 0.0f );
    windowLabelTest->VirtualSize.x = 0.40f;
    windowLabelTest->ColorAndAlpha = nyaVec4f( 1.0f, 1.0f, 1.0f, 1.0f );
    windowLabelTest->Value = "New Window";

    GUIPanel& titleBarTest = g_DebugGUI->allocatePanel();
    titleBarTest.VirtualPosition = nyaVec2f( 0.0f, 0.0f );
    titleBarTest.VirtualSize = nyaVec2f( 250.0f, 8.0f );
    titleBarTest.PanelMaterial = g_GraphicsAssetCache->getMaterial( NYA_STRING( "GameData/materials/HUD/DefaultMaterial.mat" ) );

    GUIPanel& buttonTest = g_DebugGUI->allocatePanel();
    buttonTest.VirtualPosition = nyaVec2f( 0.965f, 0.00f );
    buttonTest.VirtualSize = nyaVec2f( 8.0f, 8.0f );
    buttonTest.PanelMaterial = g_GraphicsAssetCache->getMaterial( NYA_STRING( "GameData/materials/HUD/DefaultMaterial.mat" ) );

    GUIButton* buttonTest2 = g_DebugGUI->allocateWidget<GUIButton>();
    buttonTest2->VirtualPosition = nyaVec2f( 0.01f, 0.08f );
    buttonTest2->VirtualSize = nyaVec2f( 8.0f, 8.0f );
    buttonTest2->PanelMaterial = g_GraphicsAssetCache->getMaterial( NYA_STRING( "GameData/materials/HUD/DefaultMaterial.mat" ) );
    buttonTest2->Value = &g_IsDevMenuVisible;

    GUILabel* buttonLabel = g_DebugGUI->allocateWidget<GUILabel>();
    buttonLabel->VirtualPosition = nyaVec2f( 0.05f, 0.08f );
    buttonLabel->VirtualSize.x = 0.35f;
    buttonLabel->ColorAndAlpha = nyaVec4f( 1.0f, 1.0f, 1.0f, 1.0f );
    buttonLabel->Value = "g_IsDevMenuVisible";

    g_DebugGUI->onScreenResize( ScreenSize );

    panelTest.addChildren( windowLabelTest );
    panelTest.addChildren( &titleBarTest );
    panelTest.addChildren( &buttonTest );
    panelTest.addChildren( buttonTest2 );
    panelTest.addChildren( buttonLabel );
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

        NYA_ASSERT( !cfgFilesDir.empty(), "Failed to retrieve a suitable directory for savegame storage on your system... (cfgFilesDir = %s)", cfgFilesDir.c_str() );
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

#if NYA_DEVBUILD
    g_DrawCommandBuilder->loadDebugResources( g_GraphicsAssetCache );
#endif
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

		// Update Input
		g_InputReader->onFrame( g_InputMapper );

		// Update Local Game Instance
		g_InputMapper->update( frameTime );
		g_InputMapper->clear();

        logicCounter.onFrame( frameTime );

        accumulator += static_cast<double>( frameTime );
        
        NYA_BEGIN_PROFILE_SCOPE( "Fixed-step updates" )
            while ( accumulator >= static_cast<double>( nya::editor::LOGIC_DELTA ) ) {
                g_SceneTest->updateLogic( nya::editor::LOGIC_DELTA );

                accumulator -= static_cast<double>( nya::editor::LOGIC_DELTA );
            }
        NYA_END_PROFILE_SCOPE()

        NYA_BEGIN_PROFILE_SCOPE( "Rendering" )
            g_FramerateGUILabel->Value = "Main Loop " + std::to_string( logicCounter.AvgDeltaTime ).substr( 0, 6 ) + " ms / " + std::to_string( logicCounter.MaxDeltaTime ).substr( 0, 6 ) + " ms (" + std::to_string( logicCounter.AvgFramePerSecond ).substr( 0, 6 ) + " FPS)";
            
            g_DebugGUI->collectDrawCmds( *g_DrawCommandBuilder );

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

    nya::core::free( g_GlobalAllocator, g_DrawCommandBuilder );
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
