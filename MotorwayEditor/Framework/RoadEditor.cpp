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
#include "RoadEditor.h"

#include <AppShared.h>

#include <Framework/SceneNodes/SceneNode.h>

#include <Framework/TransactionHandler/TransactionHandler.h>
#include <Framework/TransactionHandler/SceneNodeCopyCommand.h>
#include <Framework/TransactionHandler/SceneNodeDeleteCommand.h>
#include <Framework/TransactionHandler/ImGuiEditCommand.h>
#include <Core/Allocators/LinearAllocator.h>

#include <FileSystem/FileSystemObjectNative.h>

#include <Core/PathHelpers.h>
#include <Graphics/WorldRenderer.h>

#include <imgui/imgui.h>

int pointCount = 0;
glm::vec3 pointList[32];

void flan::framework::DisplayRoadEditor()
{
    ImGui::Text( "Point Count: %i", pointCount );

    // Road Profile
    //  - Material Set
    //  - Handmade chunks rules
    //  - AI profile
    //  - Mini map generation (dump beziers curves as is)
    for ( int i = 0; i < pointCount; i++ ) {
        if ( i > 0 ) {
            g_DrawCommandBuilder->addLineToRender( pointList[i - 1], pointList[i], 1.0f, glm::vec4( 1, 0, 0, 1 ) );
        }

        g_DrawCommandBuilder->addWireframeAABB( pointList[i], glm::vec3( 2.0f ) );
    }
}

void flan::framework::AddPointToRoad( const Ray& mousePickingRay, Terrain* terrain, CommandList* cmdList )
{
    FLAN_IMPORT_VAR_PTR( dev_TerrainMousePosition, glm::vec3 )

    pointList[pointCount++] = *dev_TerrainMousePosition;
}