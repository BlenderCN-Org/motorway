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
#include <Framework/RoadEditor.h>

#include <Framework/Scene.h>
#include <Framework/Material.h>

#include <Input/InputMapper.h>

#include <Core/Allocators/LinearAllocator.h>
#include <Core/BlueNoise.h>

enum class eEditorTab
{
    SANDBOX = 3,
    TERRAIN_ED = 2,
    NODE_ED = 0,
    MATERIAL_ED = 1,
    ROAD_ED = 4
};

FLAN_DEV_VAR( dev_GuizmoViewMatrix, "Transform Guizmo ViewMatrix", nullptr, float* )
FLAN_DEV_VAR( dev_GuizmoProjMatrix, "Transform Guizmo ProjectionMatrix", nullptr, float* )
FLAN_DEV_VAR( dev_EditorPickedMaterial, "Material Picked in the Material Editor", nullptr, Material* )
FLAN_DEV_VAR( panelId, "Current Editor Mode", eEditorTab::NODE_ED, eEditorTab )

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

static bool PrintTab( const char* tabName, const eEditorTab tabIndex )
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
            ImGui::SetWindowSize( ImVec2( 1600, 160 ) );

            auto previousTabIndex = panelId;

            PrintTab( "Sandbox/Debug", eEditorTab::SANDBOX );
            ImGui::SameLine( 0, 2 );
            PrintTab( "NodeEd", eEditorTab::NODE_ED );
            ImGui::SameLine( 0, 2 );
            PrintTab( "MaterialEd", eEditorTab::MATERIAL_ED );
            ImGui::SameLine( 0, 2 );

            // Trash code, but it just works
            if ( PrintTab( "TerrainEd", eEditorTab::TERRAIN_ED ) ) {
                g_InputMapper->pushContext( FLAN_STRING_HASH( "TerrainEditor" ) );
            } else if ( previousTabIndex == eEditorTab::TERRAIN_ED && previousTabIndex != panelId ) {
                g_InputMapper->popContext();
            }

            ImGui::SameLine( 0, 2 );

            if ( PrintTab( "RoadEd", eEditorTab::ROAD_ED ) ) {
                if ( previousTabIndex == eEditorTab::TERRAIN_ED && previousTabIndex != panelId ) {
                    g_InputMapper->popContext();
                }

                g_InputMapper->pushContext( FLAN_STRING_HASH( "RoadEditor" ) );

                FLAN_IMPORT_VAR_PTR( PickedNode, SceneNode* )
                
                auto worldTranslation = ( *PickedNode )->transform.getWorldTranslation();
                worldTranslation.y = 64.0f * 5.0f;

                cameraNode->camera->GetDataRW().worldPosition = worldTranslation; 
                cameraNode->camera->setOrientation( 0, -1.57f, 0 );
            } else if ( previousTabIndex == eEditorTab::ROAD_ED && previousTabIndex != panelId ) {
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
            case eEditorTab::NODE_ED:
                flan::framework::DisplayNodeEditor( frameTime );
                break;

            case eEditorTab::MATERIAL_ED:
                MaterialEd();
                break;

            case eEditorTab::TERRAIN_ED:
                flan::framework::DisplayTerrainEditor();
                break;

            case eEditorTab::SANDBOX:
                ImGui::Text( "Debug/Sandbox/Quick and dirty tests" );

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

            case eEditorTab::ROAD_ED:
                flan::framework::DisplayRoadEditor();
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
