#include <Shared.h>
#include "Editor.h"

#include "EditorShared.h"
#include "Tickrate.h"

#include <Audio/AudioDevice.h>

#include <Core/Timer.h>
#include <Core/FileLogger.h>
#include <Core/Environment.h>
#include <Core/StringHelpers.h>

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

#include <Core/ColormetryHelpers.h>

#include <Framework/TransactionHandler/TranslateCommand.h>
#include <Framework/TransactionHandler/ScaleCommand.h>
#include <Framework/TransactionHandler/RotateCommand.h>
#include <Framework/TransactionHandler/TransactionHandler.h>

#include "DefaultInputConfig.h"

#include <FileSystem/FileSystemWatchdog.h>

#if NYA_DEVBUILD
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/examples/imgui_impl_dx11.h>
#include <imgui/examples/imgui_impl_win32.h>
#include <ImGuizmo/ImGuizmo.h>
#include <Display/DisplaySurfaceWin32.h>
#include <Rendering/Direct3D11/CommandList.h>
#include <Rendering/Direct3D11/RenderDevice.h>
#include <Rendering/Direct3D11/RenderTarget.h>  
#include <d3d11.h>

#include <Core/FileSystemIOHelpers.h>

static bool IsItemActiveLastFrame()
{
    ImGuiContext& g = *ImGui::GetCurrentContext();
    if ( g.ActiveIdPreviousFrame )
        return g.ActiveIdPreviousFrame == g.CurrentWindow->DC.LastItemId;
    return false;
}

#define NYA_IMGUI_UNDO_IMPL( transactionHandler, variable, cmdType )\
    static decltype( variable ) variable##_Backup = {}; \
    if ( ImGui::IsItemActive() && !IsItemActiveLastFrame() ) {\
        variable##_Backup = variable;\
    }\
    if ( ImGui::IsItemDeactivatedAfterEdit() ) {\
        transactionHandler->commit<cmdType>( &variable, variable, variable##_Backup );\
    }

#define NYA_IMGUI_UNDO_IMPL_PTR( transactionHandler, variable, cmdType )\
    static decltype( variable ) variable##_Backup = {}; \
    if ( ImGui::IsItemActive() && !IsItemActiveLastFrame() ) {\
        variable##_Backup = variable;\
    }\
    if ( ImGui::IsItemDeactivatedAfterEdit() ) {\
        transactionHandler->commit<cmdType>( variable, *variable, variable##_Backup );\
    }

#define NYA_IMGUI_DRAG_FLOAT( transactionHandler, variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat( #variable, &variable, rangeMin, rangeMax );\
    NYA_IMGUI_UNDO_IMPL( transactionHandler, variable, FloatEditCommand )

#define NYA_IMGUI_DRAG_FLOAT_PTR( transactionHandler, variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat( #variable, variable, rangeMin, rangeMax );\
    NYA_IMGUI_UNDO_IMPL_PTR( variable, FloatEditCommand )

#define NYA_IMGUI_DRAG_INT( transactionHandler, variable, step, rangeMin, rangeMax )\
    ImGui::DragInt( #variable, &variable, step, rangeMin, rangeMax );\
    NYA_IMGUI_UNDO_IMPL( transactionHandler, variable, IntEditCommand )

#define NYA_IMGUI_DRAG_FLOAT2( transactionHandler, variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat2( #variable, &variable, step, rangeMin, rangeMax );\
    NYA_IMGUI_UNDO_IMPL( transactionHandler, variable, Float2EditCommand )

#define NYA_IMGUI_DRAG_FLOAT3( transactionHandler, variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat3( #variable, (float*)&variable, step, rangeMin, rangeMax );\
    NYA_IMGUI_UNDO_IMPL( transactionHandler, variable, Float3EditCommand )

#define NYA_IMGUI_INPUT_FLOAT3( transactionHandler, variable )\
    ImGui::InputFloat3( #variable, (float*)&variable );\
    NYA_IMGUI_UNDO_IMPL( transactionHandler, variable, Float3EditCommand )

#define NYA_IMGUI_SLIDER_INT( transactionHandler, variable, rangeMin, rangeMax )\
    ImGui::SliderInt( #variable, &variable, rangeMin, rangeMax );\
    NYA_IMGUI_UNDO_IMPL( transactionHandler, variable, IntEditCommand )

#define NYA_IMGUI_CHECKBOX( transactionHandler, variable, onChecked )\
    if ( ImGui::Checkbox( #variable, &variable ) ) {\
        onChecked\
    }\
    NYA_IMGUI_UNDO_IMPL( transactionHandler, variable, BoolEditCommand )

#define NYA_IMGUI_CHECKBOX_PTR( transactionHandler, variable, onChecked )\
    if ( ImGui::Checkbox( #variable, variable ) ) {\
        onChecked\
    }\
    NYA_IMGUI_UNDO_IMPL_PTR( variable, BoolEditCommand )

#define NYA_IMPL_EDIT_TYPE( cmdName, type )\
class cmdName##Command : public TransactionCommand\
{\
public:\
cmdName##Command( type* editedValue, type nextValue, type backupValue )\
        : editedValue( editedValue )\
        , nextValue( nextValue )\
        , backupValue( backupValue )\
    {\
        actionInfos = "Edit " #type " Variable";\
    }\
\
    virtual void execute() override\
    {\
        *editedValue = nextValue;\
    }\
\
    virtual void undo() override\
    {\
        *editedValue = backupValue;\
    }\
\
private:\
    type*      editedValue;\
    const type nextValue;\
    const type backupValue;\
};

NYA_IMPL_EDIT_TYPE( BoolEdit, bool )
NYA_IMPL_EDIT_TYPE( FloatEdit, float )
NYA_IMPL_EDIT_TYPE( IntEdit, int )
NYA_IMPL_EDIT_TYPE( Float2Edit, nyaVec2f )
NYA_IMPL_EDIT_TYPE( Float3Edit, nyaVec3f )
NYA_IMPL_EDIT_TYPE( Float4Edit, nyaVec4f )

#endif

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
static FileSystemWatchdog*     g_FileSystemWatchdog;

static Scene*                  g_SceneTest;
static FreeCamera*             g_FreeCamera;
static GUIScreen*              g_DebugGUI;
static GUILabel*               g_FramerateGUILabel;
static bool                    g_IsDevMenuVisible = false;
static Scene::Node*            g_PickedNode = nullptr;
static TransactionHandler*     g_TransactionHandler = nullptr;
static Ray                     g_PickingRay( nyaVec3f( 0, 0, 0 ), nyaVec3f( 0, 0, 0 ) );

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
            }
            else {
                g_InputMapper->popContext();
            }
        }

        if ( input.States.find( NYA_STRING_HASH( "MoveCamera" ) ) != input.States.end() ) {
            // Camera Controls
            auto axisX = input.Ranges[NYA_STRING_HASH( "CameraMoveHorizontal" )];
            auto axisY = input.Ranges[NYA_STRING_HASH( "CameraMoveVertical" )];

            g_FreeCamera->updateMouse( frameTime, axisX, axisY );
        }

        auto rawX = nya::maths::clamp( static_cast< float >( g_InputReader->getAbsoluteAxisValue( nya::input::eInputAxis::MOUSE_X ) ), 0.0f, static_cast< float >( ScreenSize.x ) );
        auto rawY = nya::maths::clamp( static_cast< float >( g_InputReader->getAbsoluteAxisValue( nya::input::eInputAxis::MOUSE_Y ) ), 0.0f, static_cast< float >( ScreenSize.y ) );

        g_DebugGUI->onMouseCoordinatesUpdate( rawX, rawY );
        if ( input.States.find( NYA_STRING_HASH( "MouseClick" ) ) != input.States.end() ) {
            g_DebugGUI->onLeftMouseButtonDown( rawX, rawY );
        }
        else {
            g_DebugGUI->onLeftMouseButtonUp();
        }
        ImGuiIO& io = ImGui::GetIO();

        // Default: Ctrl
        if ( input.States.find( NYA_STRING_HASH( "Modifier1" ) ) != input.States.end() ) {
            if ( input.Actions.find( NYA_STRING_HASH( "Undo" ) ) != input.Actions.end() ) {
                g_TransactionHandler->undo();
            }

            if ( input.Actions.find( NYA_STRING_HASH( "Redo" ) ) != input.Actions.end() ) {
                g_TransactionHandler->redo();
            }
        }

        if ( !io.WantCaptureMouse ) {
            if ( input.Actions.find( NYA_STRING_HASH( "PickNode" ) ) != input.Actions.end() ) {
                auto cameraData = g_FreeCamera->getData();
                auto viewMat = cameraData.viewMatrix;
                auto projMat = cameraData.depthProjectionMatrix;

                auto inverseViewProj = ( projMat * viewMat ).inverse();

                nyaVec4f ray =
                {
                    ( io.MousePos.x / io.DisplaySize.x ) * 2.f - 1.f,
                    ( 1.f - ( io.MousePos.y / io.DisplaySize.y ) ) * 2.f - 1.f,
                    0.0f,
                    1.0f
                };

                nyaVec4f rayOrigin =
                {
                    ray.x * inverseViewProj[0][0] + ray.y * inverseViewProj[1][0] + ray.z * inverseViewProj[2][0] + ray.w * inverseViewProj[3][0],
                    ray.x * inverseViewProj[0][1] + ray.y * inverseViewProj[1][1] + ray.z * inverseViewProj[2][1] + ray.w * inverseViewProj[3][1],
                    ray.x * inverseViewProj[0][2] + ray.y * inverseViewProj[1][2] + ray.z * inverseViewProj[2][2] + ray.w * inverseViewProj[3][2],
                    ray.x * inverseViewProj[0][3] + ray.y * inverseViewProj[1][3] + ray.z * inverseViewProj[2][3] + ray.w * inverseViewProj[3][3],
                };
                rayOrigin *= ( 1.0f / rayOrigin.w );

                ray.z = 1.0f;
                nyaVec4f rayEnd =
                {
                    ray.x * inverseViewProj[0][0] + ray.y * inverseViewProj[1][0] + ray.z * inverseViewProj[2][0] + ray.w * inverseViewProj[3][0],
                    ray.x * inverseViewProj[0][1] + ray.y * inverseViewProj[1][1] + ray.z * inverseViewProj[2][1] + ray.w * inverseViewProj[3][1],
                    ray.x * inverseViewProj[0][2] + ray.y * inverseViewProj[1][2] + ray.z * inverseViewProj[2][2] + ray.w * inverseViewProj[3][2],
                    ray.x * inverseViewProj[0][3] + ray.y * inverseViewProj[1][3] + ray.z * inverseViewProj[2][3] + ray.w * inverseViewProj[3][3],
                };
                rayEnd *= ( 1.0f / rayEnd.w );

                auto rayDir = ( rayEnd - rayOrigin ).normalize();

                nyaVec3f rayDirection = nyaVec3f( rayDir );
                nyaVec3f rayOrig = nyaVec3f( rayOrigin );

                g_PickingRay.origin = rayOrig;
                g_PickingRay.direction = rayDirection;

                g_PickedNode = g_SceneTest->intersect( g_PickingRay );
            }
        }
    }, -1 );
}

void TestStuff()
{
    NYA_CLOG << "Initializing stuff..." << std::endl;

    g_TransactionHandler = nya::core::allocate<TransactionHandler>( g_GlobalAllocator, g_GlobalAllocator );

    auto freeCameraId = g_SceneTest->FreeCameraDatabase.allocate();

    // Retrieve pointer to camera instance from scene db
    g_FreeCamera = &g_SceneTest->FreeCameraDatabase[freeCameraId];
    g_FreeCamera->setProjectionMatrix( CameraFOV, static_cast<float>( ScreenSize.x ), static_cast<float>( ScreenSize.y ) );
    
    g_FreeCamera->setImageQuality( ImageQuality );
    g_FreeCamera->setMSAASamplerCount( MSAASamplerCount );

    // Toggle camera flags based on user settings
    auto& cameraFlags = g_FreeCamera->getUpdatableFlagset();
    cameraFlags.enableTAA = EnableTAA;

    Scene::StaticGeometryNode* meshTest = g_SceneTest->allocateStaticGeometry();
    meshTest->name = "meshTest";

    auto& geometry = g_SceneTest->RenderableMeshDatabase[meshTest->mesh];
    geometry.meshResource = g_GraphicsAssetCache->getMesh( NYA_STRING( "GameData/geometry/test.mesh" ) );

    Scene::StaticGeometryNode* planeTest = g_SceneTest->allocateStaticGeometry();
    planeTest->name = "PlaneTest";

    auto& geometryPlane = g_SceneTest->RenderableMeshDatabase[planeTest->mesh];
    geometryPlane.meshResource = g_GraphicsAssetCache->getMesh( NYA_STRING( "GameData/geometry/plane.mesh" ) );
    geometryPlane.renderDepth = 0;

 /*   for ( int j = 0; j < 5; j++ ) {
        for ( int i = 0; i < 6; i++ ) {
            PointLightData pointLightData;
            pointLightData.worldPosition = { static_cast< float >( i ), 0.25f, static_cast< float>( j ) };
            pointLightData.radius = 0.50f;
            pointLightData.lightPower = 100.0f;
            pointLightData.colorRGB = { static_cast< float >( rand() ) / RAND_MAX, static_cast< float >( rand() ) / RAND_MAX, static_cast< float >( rand() ) / RAND_MAX };

            Scene::PointLightNode* pointLight = g_SceneTest->allocatePointLight();
            pointLight.pointLight = g_LightGrid->allocatePointLightData( std::forward<PointLightData>( pointLightData ) );

            auto& pointLightTransform = g_SceneTest->TransformDatabase[pointLight.transform];
            pointLightTransform.setWorldTranslation( pointLightData.worldPosition );
        }
    }*/
    
    DirectionalLightData sunLight = {};
    sunLight.isSunLight = true;
    sunLight.intensityInLux = 100000.0f;
    sunLight.angularRadius = 0.00935f / 2.0f;
    const float solidAngle = ( 2.0f * nya::maths::PI<float>() ) * static_cast<float>( 1.0f - cosf( sunLight.angularRadius ) );

    sunLight.illuminanceInLux = sunLight.intensityInLux * solidAngle;
    sunLight.sphericalCoordinates = nyaVec2f( 0.50f, 0.5f );
    sunLight.direction = nya::maths::SphericalToCarthesianCoordinates( sunLight.sphericalCoordinates.x, sunLight.sphericalCoordinates.y );

    auto* dirLight = g_SceneTest->allocateDirectionalLight();
    dirLight->dirLightData = g_LightGrid->updateDirectionalLightData( std::forward<DirectionalLightData>( sunLight ) );

   //{
   //     IBLProbeData localProbe = {};
   //     localProbe.worldPosition = { 0, 10, 2 };
   //     localProbe.radius = 4.0f;
   //     localProbe.isFallbackProbe = false;

   //     nyaMat4x4f probeModelMatrix = nya::maths::MakeTranslationMat( localProbe.worldPosition ) * nya::maths::MakeScaleMat( localProbe.radius );
   //     localProbe.inverseModelMatrix = probeModelMatrix.inverse();

   //     auto& localProbeNode = g_SceneTest->allocateIBLProbe();
   //     localProbeNode.iblProbe = g_LightGrid->allocateLocalIBLProbeData( std::forward<IBLProbeData>( localProbe ) );
   // }
   // {
   //     IBLProbeData localProbe = {};
   //     localProbe.worldPosition = { -10, 10, 2 };
   //     localProbe.radius = 4.0f;
   //     localProbe.isFallbackProbe = false;

   //     nyaMat4x4f probeModelMatrix = nya::maths::MakeTranslationMat( localProbe.worldPosition ) * nya::maths::MakeScaleMat( localProbe.radius );
   //     localProbe.inverseModelMatrix = probeModelMatrix.inverse();

   //     auto& localProbeNode = g_SceneTest->allocateIBLProbe();
   //     localProbeNode.iblProbe = g_LightGrid->allocateLocalIBLProbeData( std::forward<IBLProbeData>( localProbe ) );
   // }
   // {
   //     IBLProbeData localProbe = {};
   //     localProbe.worldPosition = { -10, 10, 12 };
   //     localProbe.radius = 4.0f;
   //     localProbe.isFallbackProbe = false;

   //     nyaMat4x4f probeModelMatrix = nya::maths::MakeTranslationMat( localProbe.worldPosition ) * nya::maths::MakeScaleMat( localProbe.radius );
   //     localProbe.inverseModelMatrix = probeModelMatrix.inverse();

   //     auto& localProbeNode = g_SceneTest->allocateIBLProbe();
   //     localProbeNode.iblProbe = g_LightGrid->allocateLocalIBLProbeData( std::forward<IBLProbeData>( localProbe ) );
   // }

    IBLProbeData globalProbe = {};
    globalProbe.isFallbackProbe = true;
    Scene::IBLProbeNode* globalProbeNode = g_SceneTest->allocateIBLProbe();
    g_SceneTest->IBLProbeDatabase[globalProbeNode->iblProbe].iblProbeData = g_LightGrid->updateGlobalIBLProbeData( std::forward<IBLProbeData>( globalProbe ) );

    g_LightGrid->setSceneBounds( nyaVec3f( 20, 20, 20 ), nyaVec3f( -20, -20, -20 ) );

    g_DebugGUI = nya::core::allocate<GUIScreen>( g_GlobalAllocator, g_GlobalAllocator ); 
    g_DebugGUI->setVirtualScreenSize( nyaVec2u( 1280u, 720u ) );
/*
    GUIPanel& panelTest = g_DebugGUI->allocatePanel();
    panelTest.VirtualPosition = nyaVec2f( 0.0f, 1.00f );
    panelTest.VirtualSize = nyaVec2f( 250.0f, 120.0f );
    panelTest.PanelMaterial = g_GraphicsAssetCache->getMaterial( NYA_STRING( "GameData/materials/HUD/DefaultMaterial.mat" ) );
   */
    g_FramerateGUILabel = g_DebugGUI->allocateWidget<GUILabel>();
    g_FramerateGUILabel->VirtualPosition = nyaVec2f( 995.0f, 0.0f );
    g_FramerateGUILabel->VirtualSize.x = 0.40f;
    g_FramerateGUILabel->ColorAndAlpha = nyaVec4f( 0.9f, 0.9f, 0.0f, 1.0f );
  /*
    GUILabel* windowLabelTest = g_DebugGUI->allocateWidget<GUILabel>();
    windowLabelTest->VirtualPosition = nyaVec2f( 0.01f, 0.0f );
    windowLabelTest->VirtualSize.x = 0.40f;
    windowLabelTest->ColorAndAlpha = nyaVec4f( 1.0f, 1.0f, 1.0f, 1.0f );
    windowLabelTest->Value = "New Window";

    GUIPanel& titleBarTest = g_DebugGUI->allocatePanel();
    titleBarTest.VirtualPosition = nyaVec2f( 640.0f, 480.0f );
    titleBarTest.VirtualSize = nyaVec2f( 250.0f, 8.0f );
    titleBarTest.IsDraggable = true;
    titleBarTest.PanelMaterial = g_GraphicsAssetCache->getMaterial( NYA_STRING( "GameData/materials/HUD/DefaultMaterial.mat" ) );

    GUIPanel& buttonTest = g_DebugGUI->allocatePanel();
    buttonTest.VirtualPosition = nyaVec2f( 0.96725f, 0.0f );
    buttonTest.VirtualSize = nyaVec2f( 8.0f, 8.0f );
    buttonTest.PanelMaterial = g_GraphicsAssetCache->getMaterial( NYA_STRING( "GameData/materials/HUD/CloseButton.mat" ) );

    GUIButton& buttonTest2 = g_DebugGUI->allocatePanel<GUIButton>();
    buttonTest2.VirtualPosition = nyaVec2f( 0.01f, 0.01f );
    buttonTest2.VirtualSize = nyaVec2f( 8.0f, 8.0f );
    buttonTest2.PanelMaterial = g_GraphicsAssetCache->getMaterial( NYA_STRING( "GameData/materials/HUD/Checkbox.mat" ) );
    buttonTest2.Value = &EnableVSync;
    
    GUILabel* buttonLabel = g_DebugGUI->allocateWidget<GUILabel>();
    buttonLabel->VirtualPosition = nyaVec2f( 0.05f, 0.01f );
    buttonLabel->VirtualSize.x = 0.35f;
    buttonLabel->ColorAndAlpha = nyaVec4f( 1.0f, 1.0f, 1.0f, 1.0f );
    buttonLabel->Value = "EnableVSync";
    g_CheckboxTestLabel = buttonLabel;
*/
    g_DebugGUI->onScreenResize( ScreenSize );

    //titleBarTest.addChild( &panelTest );
    //titleBarTest.addChild( windowLabelTest );
    //titleBarTest.addChild( &buttonTest );
    //panelTest.addChild( &buttonTest2 );
    //panelTest.addChild( buttonLabel );

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init( g_DisplaySurface->nativeDisplaySurface->Handle );

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

#if NYA_DEVBUILD
    g_FileSystemWatchdog = nya::core::allocate<FileSystemWatchdog>( g_GlobalAllocator );
    g_FileSystemWatchdog->create();
#endif
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

#if NYA_DEVBUILD
static int g_ActivePanelIndex = 0;

static void PrintTab( const char* tabName, const int tabIndex )
{
    if ( g_ActivePanelIndex == tabIndex ) {
        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.96f, 0.62f, 0.1f, 1.0f ) );
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.1f, 0.1f, 0.1f, 1.0f ) );
        if ( ImGui::Button( tabName ) ) {
            g_ActivePanelIndex = tabIndex;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }
    else {
        if ( ImGui::Button( tabName ) ) {
            g_ActivePanelIndex = tabIndex;
        }
    }
}

static void PrintNode( Scene::Node* node )
{
    ImGuiTreeNodeFlags flags = 0;

    const bool isLeaf = node->children.empty();
    const bool isSelected = ( g_PickedNode == node );

    if ( isSelected )
        flags |= ImGuiTreeNodeFlags_Selected;

    if ( isLeaf )
        flags |= ImGuiTreeNodeFlags_Leaf;

    bool isOpened = ImGui::TreeNodeEx( node->name.c_str(), flags );

    if ( ImGui::IsItemClicked() ) {
        g_PickedNode = node;
    }

    if ( isOpened ) {
        if ( !isLeaf ) {
            ImGui::Indent();
            for ( auto child : node->children ) {
                PrintNode( child );
            }
        }

        ImGui::TreePop();
    }
}

static void DisplayMenuBar()
{
    if ( ImGui::BeginMainMenuBar() ) {
            if ( ImGui::BeginMenu( "File" ) ) {
                if ( ImGui::MenuItem( "New Scene" ) ) {

                }

                if ( ImGui::MenuItem( "Load Scene..." ) ) {

                }

                if ( ImGui::MenuItem( "Save Scene..." ) ) {

                }

                if ( ImGui::MenuItem( "Export Scene..." ) ) {

                }

                ImGui::EndMenu();
            }

            if ( ImGui::BeginMenu( "Edit" ) ) {
                if ( ImGui::MenuItem( "Undo" ) ) {
                    g_TransactionHandler->undo();
                }

                if ( ImGui::MenuItem( "Redo" ) ) {
                    g_TransactionHandler->redo();
                }

                ImGui::Separator();

                if ( ImGui::MenuItem( "Copy" ) ) {

                }

                if ( ImGui::MenuItem( "Paste" ) ) {

                }

                if ( ImGui::MenuItem( "Delete" ) ) {

                }
                ImGui::EndMenu();
            }

            if ( ImGui::BeginMenu( "Add To Scene" ) ) {
                if ( ImGui::MenuItem( "Static Geometry" ) ) {
                    g_PickedNode = g_SceneTest->allocateStaticGeometry();
                }

                ImGui::Separator();

                if ( ImGui::MenuItem( "Point Light" ) ) {
                    PointLightData pointLightData;
                    pointLightData.worldPosition = { 0, 1, 0 };
                    pointLightData.radius = 1.0f;
                    pointLightData.lightPower = 100.0f;
                    pointLightData.colorRGB = { 1, 1, 1 };

                    Scene::PointLightNode* pointLight = g_SceneTest->allocatePointLight();
                    g_SceneTest->PointLightDatabase[pointLight->pointLight].pointLightData = g_LightGrid->allocatePointLightData( std::forward<PointLightData>( pointLightData ) );

                    auto& pointLightTransform = g_SceneTest->TransformDatabase[pointLight->transform];
                    pointLightTransform.setWorldTranslation( pointLightData.worldPosition );

                    g_PickedNode = pointLight;
                }

                if ( ImGui::MenuItem( "IBL Probe" ) ) {
                    IBLProbeData localProbe = {};
                    localProbe.worldPosition = { 0, 4, 0 };
                    localProbe.radius = 4.0f;
                    localProbe.isFallbackProbe = false;

                    nyaMat4x4f probeModelMatrix = nya::maths::MakeTranslationMat( localProbe.worldPosition ) * nya::maths::MakeScaleMat( localProbe.radius );
                    localProbe.inverseModelMatrix = probeModelMatrix.inverse();

                    Scene::IBLProbeNode* localProbeNode = g_SceneTest->allocateIBLProbe();
                    g_SceneTest->IBLProbeDatabase[localProbeNode->iblProbe].iblProbeData = g_LightGrid->allocateLocalIBLProbeData( std::forward<IBLProbeData>( localProbe ) );
                
                    g_PickedNode = localProbeNode;
                }

                ImGui::EndMenu();
            }

            if ( ImGui::BeginMenu( "Graphics" ) ) {
                if ( ImGui::BeginMenu( "MSAA" ) ) {
                    if ( ImGui::MenuItem( "x1", nullptr, MSAASamplerCount == 1 ) ) {
                        MSAASamplerCount = 1;
                        g_FreeCamera->setMSAASamplerCount( MSAASamplerCount );
                    }
                    if ( ImGui::MenuItem( "x2", nullptr, MSAASamplerCount == 2 ) ) {
                        MSAASamplerCount = 2;
                        g_FreeCamera->setMSAASamplerCount( MSAASamplerCount );
                    }
                    if ( ImGui::MenuItem( "x4", nullptr, MSAASamplerCount == 4 ) ) {
                        MSAASamplerCount = 4;
                        g_FreeCamera->setMSAASamplerCount( MSAASamplerCount );
                    }
                    if ( ImGui::MenuItem( "x8", nullptr, MSAASamplerCount == 8 ) ) {
                        MSAASamplerCount = 8;
                        g_FreeCamera->setMSAASamplerCount( MSAASamplerCount );
                    }
                    ImGui::EndMenu();
                }

                if ( ImGui::SliderFloat( "Image Quality", &ImageQuality, 0.01f, 4.0f ) ) {
                    g_FreeCamera->setImageQuality( ImageQuality );
                }

                if ( ImGui::Checkbox( "Enable VSync", &EnableVSync ) ) {
                    g_RenderDevice->enableVerticalSynchronisation( EnableVSync );
                }

                if ( ImGui::Checkbox( "Enable Temporal AntiAliasing", &EnableTAA ) ) {
                    auto& cameraFlags = g_FreeCamera->getUpdatableFlagset();
                    cameraFlags.enableTAA = EnableTAA;
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
}

void PrintEditorGUI()
{
    if ( !g_IsDevMenuVisible ) {
        return;
    }

    CommandList& cmdList = g_RenderDevice->allocateGraphicsCommandList();
    ImGui_ImplDX11_Init( g_RenderDevice->renderContext->nativeDevice, cmdList.CommandListObject->deferredContext );
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    {
        cmdList.CommandListObject->deferredContext->OMSetRenderTargets( 1, &g_RenderDevice->getSwapchainBuffer()->textureRenderTargetView, nullptr );

        ImGuizmo::BeginFrame();

        DisplayMenuBar();

        ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0, 0, 0, 0 ) );
        ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0, 0, 0, 0 ) );
        ImGui::PushStyleColor( ImGuiCol_BorderShadow, ImVec4( 0, 0, 0, 0 ) );
        if ( ImGui::Begin( "TabManager", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) ) {
            ImGui::SetWindowPos( ImVec2( 16, 48 ) );
            ImGui::SetWindowSize( ImVec2( 800, 40 ) );
            PrintTab( "NodeEd", 0 );
            ImGui::SameLine( 0, 2 );
            PrintTab( "MaterialEd", 1 );

            ImGui::End();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::PushStyleVar( ImGuiStyleVar_::ImGuiStyleVar_WindowRounding, 0 );
        if ( ImGui::Begin( "Editor Panel", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) ) {
            ImGui::SetWindowPos( ImVec2( 16, 75 ) );
            ImGui::SetWindowSize( ImVec2( 800, 390 ) );

            if ( g_PickedNode != nullptr ) {
                ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.96f, 0.1f, 0.05f, 1.0f ) );
                if ( ImGui::Button( "Delete!" ) ) {
                    ImGui::PopStyleColor();
                }
                else {
                    ImGui::PopStyleColor();
                    if ( ImGui::InputText( "Name", &g_PickedNode->name[0], 256, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue ) ) {
                        //*dev_IsInputText = !*dev_IsInputText;

                        g_PickedNode->hashcode = nya::core::CRC32( g_PickedNode->name );
                    }

                    if ( ImGui::IsItemClicked() ) {
                        //*dev_IsInputText = !*dev_IsInputText;
                    }

                    if ( ImGui::TreeNode( "Transform" ) ) {
                        Transform& transform = g_SceneTest->TransformDatabase[g_PickedNode->transform];

                        static ImGuizmo::OPERATION mCurrentGizmoOperation( ImGuizmo::TRANSLATE );
                        static int activeManipulationMode = 0;
                        static bool useSnap = false;
                        static float snap[3] = { 1.f, 1.f, 1.f };

                        nyaMat4x4f* modelMatrix = transform.getLocalModelMatrix();

                        ImGui::Checkbox( "", &useSnap );
                        ImGui::SameLine();

                        switch ( mCurrentGizmoOperation ) {
                        case ImGuizmo::TRANSLATE:
                            ImGui::InputFloat3( "Snap", snap );
                            break;
                        case ImGuizmo::ROTATE:
                            ImGui::InputFloat( "Angle Snap", snap );
                            break;
                        case ImGuizmo::SCALE:
                            ImGui::InputFloat( "Scale Snap", snap );
                            break;
                        }

                        ImGui::RadioButton( "Translate", &activeManipulationMode, 0 );
                        ImGui::SameLine();
                        ImGui::RadioButton( "Rotate", &activeManipulationMode, 1 );
                        ImGui::SameLine();
                        ImGui::RadioButton( "Scale", &activeManipulationMode, 2 );

                        CameraData& cameraData = g_FreeCamera->getData();
                        ImGuiIO& io = ImGui::GetIO();
                        ImGuizmo::SetRect( 0, 0, io.DisplaySize.x, io.DisplaySize.y );
                        ImGuizmo::Manipulate( cameraData.viewMatrix.toArray(), cameraData.depthProjectionMatrix.toArray(), static_cast< ImGuizmo::OPERATION >( activeManipulationMode ), ImGuizmo::MODE::LOCAL, modelMatrix->toArray(), NULL, useSnap ? &snap[activeManipulationMode] : NULL );

                        nyaVec3f Translation = nya::maths::ExtractTranslation( *modelMatrix );
                        NYA_IMGUI_INPUT_FLOAT3( g_TransactionHandler, Translation )

                        nyaVec3f Scale = nya::maths::ExtractScale( *modelMatrix );

                        nyaQuatf RotationQuat = nya::maths::ExtractRotation( *modelMatrix, Scale );
                        nyaVec3f Rotation = RotationQuat.toEulerAngles();
                        ImGui::InputFloat3( "Rotation", ( float* )&Rotation, 3 );
                        RotationQuat = nyaQuatf( Rotation );

                        static nyaVec3f Rotation_Backup = {};
                        if ( ImGui::IsItemActive() && !IsItemActiveLastFrame() ) {
                            Rotation_Backup = Rotation;
                        }
                        if ( ImGui::IsItemDeactivatedAfterEdit() ) {
                            g_TransactionHandler->commit<LocalRotateCommand>( &transform, RotationQuat );
                            transform.setLocalRotation( RotationQuat );
                        }

                        NYA_IMGUI_INPUT_FLOAT3( g_TransactionHandler, Scale )

                        if ( ImGuizmo::IsUsing() ) {
                            g_TransactionHandler->commit<LocalTranslateCommand>( &transform, Translation );
                            g_TransactionHandler->commit<LocalScaleCommand>( &transform, Scale );
                            g_TransactionHandler->commit<LocalRotateCommand>( &transform, RotationQuat );
                        } else {
                            transform.setLocalTranslation( Translation );
                            transform.setLocalScale( Scale );
                        }

                        ImGui::TreePop();
                    }

                    switch ( g_PickedNode->getNodeType() ) {
                    case NYA_STRING_HASH( "IBLProbeNode" ):
                    {
                        if ( ImGui::TreeNode( "IBL Probe" ) ) {
                            Scene::IBLProbeNode* sceneNode = static_cast< Scene::IBLProbeNode* >( g_PickedNode );
                            IBLProbeData* probeData = ( *sceneNode->iblProbeData );

                            if ( ImGui::Button( "Force Probe Capture" ) ) {
                                probeData->isCaptured = false;
                            }

                            ImGui::DragFloat( "Radius", &probeData->radius, 0.01f, 0.01f, 64.0f );

                            ImGui::Checkbox( "Is Fallback Probe", &probeData->isFallbackProbe );
                            ImGui::Checkbox( "Is Dynamic", &probeData->isDynamic );

                            if ( probeData->isDynamic ) {
                                ImGui::InputInt( "Update Frequency (in frames)", ( int* )&probeData->CaptureFrequency );
                            }

                            ImGui::TreePop();
                        }
                    } break;

                    case NYA_STRING_HASH( "DirectionalLightNode" ):
                    {
                        if ( ImGui::TreeNode( "Directional Light" ) ) {
                            Scene::DirectionalLightNode* sceneNode = static_cast< Scene::DirectionalLightNode* >( g_PickedNode );
                            DirectionalLightData* dirLightData = sceneNode->dirLightData;

                            ImGui::Checkbox( "Enable Shadow", &dirLightData->enableShadow );
                            ImGui::SameLine();
                            ImGui::Checkbox( "Acts as Sun", &dirLightData->isSunLight );

                            nya::editor::PanelLuminousIntensity( dirLightData->intensityInLux );
                            nya::editor::PanelColor( sceneNode->colorMode, dirLightData->colorRGB );

                            ImGui::DragFloat( "Angular Radius", &dirLightData->angularRadius, 0.00001f, 0.0f, 1.0f );

                            const float solidAngle = ( 2.0f * nya::maths::PI<float>() ) * ( 1.0f - cos( dirLightData->angularRadius ) );

                            dirLightData->illuminanceInLux = dirLightData->intensityInLux * solidAngle;

                            ImGui::DragFloat( "Spherical Coordinate Theta", &dirLightData->sphericalCoordinates.x, 0.01f, -1.0f, 1.0f );
                            ImGui::DragFloat( "Spherical Coordinate Gamma", &dirLightData->sphericalCoordinates.y, 0.01f, -1.0f, 1.0f );

                            dirLightData->direction = nya::maths::SphericalToCarthesianCoordinates( dirLightData->sphericalCoordinates.x, dirLightData->sphericalCoordinates.y );

                            ImGui::TreePop();
                        }
                    } break;

                    case NYA_STRING_HASH( "PointLightNode" ):
                    {
                        if ( ImGui::TreeNode( "Point Light" ) ) {
                            Scene::PointLightNode* sceneNode = static_cast< Scene::PointLightNode* >( g_PickedNode );
                            PointLightData* pointLightData = ( *sceneNode->pointLightData );

                            ImGui::DragFloat( "Radius", &pointLightData->radius, 0.01f, 0.01f, 64.0f );

                            nya::editor::PanelLuminousIntensity( pointLightData->lightPower );
                            nya::editor::PanelColor( sceneNode->colorMode, pointLightData->colorRGB );

                            ImGui::TreePop();
                        }
                    } break;
                    
                    case NYA_STRING_HASH( "StaticGeometryNode" ):
                    {
                        if ( ImGui::TreeNode( "Static Geometry" ) ) {
                            Scene::StaticGeometryNode* sceneNode = static_cast< Scene::StaticGeometryNode* >( g_PickedNode );
                            Scene::RenderableMesh* renderableMesh = sceneNode->renderableMesh;
                            Mesh* meshResource = renderableMesh->meshResource;

                            auto meshPath = ( meshResource != nullptr ) ? nya::core::WideStringToString( meshResource->getName() ) : "(empty)";
                            ImGui::LabelText( "##meshPath", meshPath.c_str() );
                            ImGui::SameLine();

                            if ( ImGui::Button( "..." ) ) {
                                nyaString_t meshName;
                                if ( nya::core::DisplayFileOpenPrompt( meshName, NYA_STRING( "Mesh file (*.mesh)\0*.mesh" ), NYA_STRING( "./" ), NYA_STRING( "Select a Mesh" ) ) ) {
                                    meshName = nyaString_t( meshName.c_str() );

                                    auto workingDir = nyaString_t( NYA_STRING( "" ) );
                                    nya::core::RetrieveWorkingDirectory( workingDir );

                                    workingDir.append( NYA_STRING( "data" ) );
                                    size_t poswd = meshName.find( workingDir );

                                    if ( poswd != nyaString_t::npos ) {
                                        // If found then erase it from string
                                        meshName.erase( poswd, workingDir.length() );
                                    }

                                    std::replace( meshName.begin(), meshName.end(), '\\', '/' );

                                    renderableMesh->meshResource = g_GraphicsAssetCache->getMesh( ( NYA_STRING( "GameData" ) + meshName ).c_str() );
                                }
                            }

                            for ( int lodIdx = 0; lodIdx < meshResource->getLevelOfDetailCount(); lodIdx++ ) {
                                const auto& lod = meshResource->getLevelOfDetailByIndex( lodIdx );

                                if ( lod.startDistance < 0.0f ) {
                                    continue;
                                }

                                if ( ImGui::TreeNode( std::string( "LOD" + std::to_string( lodIdx ) ).c_str() ) ) {
                                    ImGui::LabelText( "##loddistance", std::string( "Distance: " + std::to_string( lod.lodDistance ) ).c_str() );

                                    ImGui::TreePop();
                                }
                            }

                            bool IsVisible = renderableMesh->isVisible;
                            NYA_IMGUI_CHECKBOX( g_TransactionHandler, IsVisible );
                            renderableMesh->isVisible = IsVisible;

                            bool RenderDepth = renderableMesh->renderDepth;
                            NYA_IMGUI_CHECKBOX( g_TransactionHandler, RenderDepth );
                            renderableMesh->renderDepth = RenderDepth;

                            ImGui::TreePop();
                        }
                    } break;
                    }
                }
            }

            ImGui::End();
        }
        if ( ImGui::Begin( "Scene Hiearchy", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) ) {
            ImGui::SetWindowPos( ImVec2( 16, 464 ) );
            ImGui::SetWindowSize( ImVec2( 800, 220 ) );

            ImGui::Text( "Scene Hiearchy" );
            ImGui::SameLine( 0, 0 );
            char sceneNodeSearch[256] = { '\0' };
            ImGui::InputText( "##SceneNodeLookup", sceneNodeSearch, 256, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue );
            ImGui::Separator();

            for ( Scene::Node* node : g_SceneTest->getNodes() ) {
                PrintNode( node );
            }

            ImGui::End();
        }
        ImGui::PopStyleVar();
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
    cmdList.end();
    g_RenderDevice->submitCommandList( &cmdList );
}
#endif

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

        g_FileSystemWatchdog->onFrame( g_GraphicsAssetCache, g_ShaderCache );

        logicCounter.onFrame( frameTime );

        accumulator += static_cast<double>( frameTime );
        
        NYA_BEGIN_PROFILE_SCOPE( "Fixed-step updates" )
            while ( accumulator >= static_cast<double>( nya::editor::LOGIC_DELTA ) ) {
                g_SceneTest->updateLogic( nya::editor::LOGIC_DELTA );

                accumulator -= static_cast<double>( nya::editor::LOGIC_DELTA );
            }
        NYA_END_PROFILE_SCOPE()

        NYA_BEGIN_PROFILE_SCOPE( "Rendering" )
            // Update Debug GUI widgets
            g_FramerateGUILabel->Value = "Main Loop " + std::to_string( logicCounter.AvgDeltaTime ).substr( 0, 6 ) + " ms / " + std::to_string( logicCounter.MaxDeltaTime ).substr( 0, 6 ) + " ms (" + std::to_string( logicCounter.AvgFramePerSecond ).substr( 0, 6 ) + " FPS)";
            g_DebugGUI->collectDrawCmds( *g_DrawCommandBuilder );

            const std::string& profileString = g_Profiler.getProfilingSummaryString();
            g_WorldRenderer->TextRenderModule->addOutlinedText( profileString.c_str(), 0.350f, 256.0f, 0.0f );
            g_WorldRenderer->LineRenderModule->addLine( g_PickingRay.origin, g_PickingRay.direction, 10.0f, nyaVec4f( 1, 0, 0, 1 ) );

            g_SceneTest->collectDrawCmds( *g_DrawCommandBuilder );

            // Update scene bounds each frame
            const AABB& sceneAabb = g_SceneTest->getSceneAabb();
            g_LightGrid->setSceneBounds( sceneAabb.maxPoint, sceneAabb.minPoint );

            g_DrawCommandBuilder->buildRenderQueues( g_WorldRenderer, g_LightGrid );

            g_WorldRenderer->drawWorld( g_RenderDevice, frameTime );
        NYA_END_PROFILE_SCOPE()

#if NYA_DEVBUILD
        PrintEditorGUI();
#endif

        g_RenderDevice->present();
    }
}

void Shutdown()
{
#if NYA_DEVBUILD
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif

    g_LightGrid->destroy( g_RenderDevice );
    g_WorldRenderer->destroy( g_RenderDevice );
    g_GraphicsAssetCache->destroy();

    nya::display::DestroyDisplaySurface( g_DisplaySurface );

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
