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
#include "EditorInterface.h"

#include <AppShared.h>

#ifdef FLAN_DEVBUILD
#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_win32.h>

#if FLAN_D3D11
#include <Rendering/Direct3D11/CommandList.h>
#include <Rendering/Direct3D11/CommandListPool.h>
#include <Rendering/Direct3D11/RenderContext.h>
#include <imgui/examples/imgui_impl_dx11.h>
#elif FLAN_VULKAN
#include <Rendering/Vulkan/CommandList.h>
#include <Rendering/Vulkan/CommandListPool.h>
#include <Rendering/Vulkan/RenderContext.h>
#include <imgui/examples/imgui_impl_vulkan.h>
#elif FLAN_GL460
#include <Rendering/OpenGL460/CommandList.h>
#include <Rendering/OpenGL460/CommandListPool.h>
#include <Rendering/OpenGL460/RenderContext.h>
#include <imgui/examples/imgui_impl_opengl3.h>
#endif

#include <Rendering/CommandListPool.h>
#include <Rendering/CommandList.h>
#include <Rendering/RenderDevice.h>

#include <Framework/SceneNodes/SceneNode.h>

#include <Framework/NodeEditor.h>
#include <Framework/EditorMenuBar.h>
#include <Framework/TerrainEditor.h>

#include <Framework/Scene.h>
#include <Framework/Material.h>

#include <Input/InputMapper.h>

#include <Core/Allocators/LinearAllocator.h>
#include <Core/BlueNoise.h>

#include <FileSystem/VirtualFileSystem.h>
#include <Io/Wave.h>
#include <Audio/AudioBuffer.h>
#include <Audio/AudioSource.h>

FLAN_DEV_VAR( dev_GuizmoViewMatrix, "Transform Guizmo ViewMatrix", nullptr, float* )
FLAN_DEV_VAR( dev_GuizmoProjMatrix, "Transform Guizmo ProjectionMatrix", nullptr, float* )
FLAN_DEV_VAR( dev_EditorPickedMaterial, "Material Picked in the Material Editor", nullptr, Material* )
FLAN_DEV_VAR( panelId, "Current Editor Mode", 0, int )

static void PrintNode( SceneNode* node )
{
    FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* )
    ImGuiTreeNodeFlags flags = 0;

    const bool isLeaf = node->children.empty();
    const bool isSelected = ( ( *PickedNode ) != nullptr && ( *PickedNode )->hashcode == node->hashcode );

    if ( isSelected ) 
        flags |= ImGuiTreeNodeFlags_Selected;

    if ( isLeaf ) 
        flags |= ImGuiTreeNodeFlags_Leaf;

    bool isOpened = ImGui::TreeNodeEx( node->name.c_str(), flags );

    if ( ImGui::IsItemClicked() ) {
        *PickedNode = node;
    }

    if ( isOpened ) {
        if ( !isLeaf ) {
            ImGui::Indent();
            for ( auto child : node->children ) {
                PrintNode( child );
            }
            ImGui::Unindent();
        }

        ImGui::TreePop();
    }
}

static bool PrintTab( const char* tabName, const int tabIndex )
{
    bool hasChanged = false;
    if ( panelId == tabIndex ) {
        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.96f, 0.62f, 0.1f, 1.0f ) );
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.1f, 0.1f, 0.1f, 1.0f ) );
        if ( ImGui::Button( tabName ) ) {
            panelId = tabIndex;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    } else {
        if ( hasChanged = ImGui::Button( tabName ) ) {
            panelId = tabIndex;
        }
    }

    return hasChanged;
}

void MaterialEd()
{
    if ( dev_EditorPickedMaterial != nullptr ) {
        dev_EditorPickedMaterial->drawInEditor( g_RenderDevice, g_ShaderStageManager, g_GraphicsAssetManager, g_WorldRenderer );
    } else {
        if ( ImGui::Button( "New" ) ) {
            dev_EditorPickedMaterial = new Material();
        }
    }
}

void flan::framework::DrawEditorInterface( const float frameTime, CommandList* cmdList )
{
    const auto& nativeContext = g_RenderDevice->getNativeRenderContext();
    const auto nativeCmdList = cmdList->getNativeCommandList();
    cmdList->beginCommandList( g_RenderDevice );
    cmdList->bindBackbufferCmd();
    
    ImGui_ImplWin32_NewFrame();

#if FLAN_D3D11
    ImGui_ImplDX11_Init( nativeContext->nativeDevice, nativeCmdList->deferredContext );
    ImGui_ImplDX11_NewFrame();
#elif FLAN_GL460
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplOpenGL3_NewFrame();
#elif FLAN_VULKAN
    //ImGui_ImplVulkan_Init( nativeContext->nativeDevice, nativeCmdList->deferredContext );
    ImGui_ImplVulkan_NewFrame();
#endif

    ImGui::NewFrame();

    FLAN_IMPORT_VAR_PTR( IsDevMenuVisible, bool )

    if ( *IsDevMenuVisible ) {
        ImGuizmo::BeginFrame();

        flan::framework::DisplayEditorMenuBar();

        // Update Guizmo Matrices
        auto cameraNode = (FreeCameraSceneNode*)g_CurrentScene->findNodeByHashcode( FLAN_STRING_HASH( "DefaultCamera" ) );
        if ( cameraNode != nullptr ) {
            auto cameraData = cameraNode->camera->GetData();
            auto transposedView = glm::transpose( cameraData.viewMatrix );
            dev_GuizmoViewMatrix = &transposedView[0][0];
            dev_GuizmoProjMatrix = &cameraData.depthProjectionMatrix[0][0];
        }

        ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0, 0, 0, 0 ) );
        ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0, 0, 0, 0 ) );
        ImGui::PushStyleColor( ImGuiCol_BorderShadow, ImVec4( 0, 0, 0, 0 ) );
        if ( ImGui::Begin( "TabManager", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) ) {
            ImGui::SetWindowPos( ImVec2( 16, 48 ) );
            ImGui::SetWindowSize( ImVec2( 800, 40 ) );

            auto previousTabIndex = panelId;

            PrintTab( "Sandbox/Debug", 3 );
            ImGui::SameLine( 0, 2 );
            PrintTab( "NodeEd", 0 );
            ImGui::SameLine( 0, 2 );
            PrintTab( "MaterialEd", 1 );
            ImGui::SameLine( 0, 2 );

            // Trash code, but it just works
            if ( PrintTab( "TerrainEd", 2 ) ) {
                g_InputMapper->pushContext( FLAN_STRING_HASH( "TerrainEditor" ) );
            } else if ( previousTabIndex == 2 && previousTabIndex != panelId ) {
                g_InputMapper->popContext();
            }

            ImGui::End();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::PushStyleVar( ImGuiStyleVar_::ImGuiStyleVar_WindowRounding, 0 );
        if ( ImGui::Begin( "Editor Panel", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) ) {
            ImGui::SetWindowPos( ImVec2( 16, 75 ) );
            ImGui::SetWindowSize( ImVec2( 800, 390 ) );
            switch ( panelId ) {
            case 0:
                flan::framework::DisplayNodeEditor( frameTime );
                break;

            case 1:
                MaterialEd();
                break;

            case 2:
                flan::framework::DisplayTerrainEditor();
                break;

            case 3:
                ImGui::Text( "Debug/Sandbox/Quick and dirty tests" );

                if ( ImGui::Button( "Sound stereo test" ) ) {
                    auto waveFile = g_VirtualFileSystem->openFile( FLAN_STRING( "GameData/Sounds/test_stereo.wav" ), flan::core::eFileOpenMode::FILE_OPEN_MODE_BINARY | flan::core::eFileOpenMode::FILE_OPEN_MODE_READ );

                    Wave waveContent;
                    flan::core::LoadWaveFile( waveFile, waveContent );

                    std::unique_ptr<AudioBuffer> audioBuffer( new AudioBuffer() );
                    audioBuffer->create( g_AudioDevice, g_GlobalAllocator );
                    audioBuffer->update( g_AudioDevice, waveContent.data.data(), waveContent.data.size(), waveContent.audioFormat, waveContent.sampleRate );

                    std::unique_ptr<AudioSource> audioSource( new AudioSource() );
                    audioSource->create( g_AudioDevice, g_GlobalAllocator );
                    audioSource->bindBuffer( g_AudioDevice, audioBuffer.get() );
                    audioSource->setPosition( g_AudioDevice, glm::vec3( 0, 0, 0 ) );
                    audioSource->setGain( g_AudioDevice, 1.0f );
                    audioSource->setPitch( g_AudioDevice, 1.0f );
                    audioSource->setLooping( g_AudioDevice, false );
                    audioSource->play( g_AudioDevice );

                    audioSource->destroy( g_AudioDevice );
                    audioBuffer->destroy( g_AudioDevice );
                }

                if ( ImGui::Button( "Blue Noise generation test" ) ) {
                    std::vector<float> texels;
                    flan::core::ComputeBlueNoise( 128, 128, texels );

                    TextureDescription noiseTest;
                    noiseTest.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
                    noiseTest.width = 128;
                    noiseTest.height = 128;
                    noiseTest.format = IMAGE_FORMAT_R32_FLOAT;
                    noiseTest.arraySize = 1;
                    noiseTest.depth = 1;
                    noiseTest.mipCount = 1;
                    noiseTest.samplerCount = 1;

                    Texture* noiseTex = flan::core::allocate<Texture>( g_GlobalAllocator );
                    noiseTex->createAsTexture2D( g_RenderDevice, noiseTest, texels.data(), texels.size() );
                }
                break;
            }
        }
        
        if ( ImGui::Begin( "Scene Hiearchy", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) ) {
            ImGui::SetWindowPos( ImVec2( 16, 464 ) );
            ImGui::SetWindowSize( ImVec2( 800, 220 ) );

            ImGui::Text( "Scene Hiearchy" );
            ImGui::SameLine( 0, 0 );
            char sceneNodeSearch[256] = { '\0' };
            ImGui::InputText( "##SceneNodeLookup", sceneNodeSearch, 256, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue );
            ImGui::Separator();

            auto scene = g_CurrentScene;
            const auto& sceneNodes = scene->getSceneNodes();
            for ( auto& node : sceneNodes ) {
                if ( node->parent == nullptr ) {
                    PrintNode( node );
                }
            }

            ImGui::End();
        }
        ImGui::PopStyleVar();

        ImGui::End();
    }

    ImGui::Render();

#if FLAN_D3D11
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
#elif FLAN_GL460
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
#elif FLAN_VULKAN
    auto nativeCmd = cmdList->getNativeCommandList();
    //ImGui_ImplVulkan_RenderDrawData( ImGui::GetDrawData(), cmdList );
#endif

    cmdList->endCommandList( g_RenderDevice );
    cmdList->playbackCommandList( g_RenderDevice );
}
#endif
