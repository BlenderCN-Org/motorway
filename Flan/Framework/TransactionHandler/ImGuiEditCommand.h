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
#pragma once

#include "TransactionCommand.h"
#include <Core/Allocators/LinearAllocator.h>
#include <imgui_internal.h>

static bool IsItemActiveLastFrame()
{
    ImGuiContext& g = *ImGui::GetCurrentContext();
    if ( g.ActiveIdPreviousFrame )
        return g.ActiveIdPreviousFrame == g.CurrentWindow->DC.LastItemId;
    return false;
}

#define FLAN_IMGUI_UNDO_IMPL( variable, cmdType )\
    static decltype( variable ) variable##_Backup = 0; \
    if ( ImGui::IsItemActive() && !IsItemActiveLastFrame() ) {\
        variable##_Backup = variable;\
    }\
    if ( ImGui::IsItemDeactivatedAfterEdit() ) {\
        g_TransactionHandler->commit( flan::core::allocate<cmdType>( g_GlobalAllocator, &variable, variable, variable##_Backup ) );\
    }

#define FLAN_IMGUI_UNDO_IMPL_PTR( variable, cmdType )\
    static decltype( variable ) variable##_Backup = 0; \
    if ( ImGui::IsItemActive() && !IsItemActiveLastFrame() ) {\
        variable##_Backup = variable;\
    }\
    if ( ImGui::IsItemDeactivatedAfterEdit() ) {\
        g_TransactionHandler->commit( flan::core::allocate<cmdType>( g_GlobalAllocator, variable, *variable, variable##_Backup ) );\
    }

#define FLAN_IMGUI_DRAG_FLOAT( variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat( #variable, &variable, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL( variable, FloatEditCommand )

#define FLAN_IMGUI_DRAG_FLOAT_PTR( variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat( #variable, variable, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL_PTR( variable, FloatEditCommand )

#define FLAN_IMGUI_DRAG_INT( variable, step, rangeMin, rangeMax )\
    ImGui::DragInt( #variable, &variable, step, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL( variable, IntEditCommand )

#define FLAN_IMGUI_DRAG_FLOAT2( variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat2( #variable, &variable, step, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL( variable, Float2EditCommand )

#define FLAN_IMGUI_DRAG_FLOAT3( variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat3( #variable, (float*)&variable, step, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL( variable, Float3EditCommand )

#define FLAN_IMGUI_SLIDER_INT( variable, rangeMin, rangeMax )\
    ImGui::SliderInt( #variable, &variable, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL( variable, IntEditCommand )

#define FLAN_IMGUI_CHECKBOX( variable, onChecked )\
    if ( ImGui::Checkbox( #variable, &variable ) ) {\
        onChecked\
    }\
    FLAN_IMGUI_UNDO_IMPL( variable, BoolEditCommand )

#define FLAN_IMGUI_CHECKBOX_PTR( variable, onChecked )\
    if ( ImGui::Checkbox( #variable, variable ) ) {\
        onChecked\
    }\
    FLAN_IMGUI_UNDO_IMPL_PTR( variable, BoolEditCommand )

#define FLAN_IMPL_EDIT_TYPE( cmdName, type )\
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

FLAN_IMPL_EDIT_TYPE( BoolEdit, bool )
FLAN_IMPL_EDIT_TYPE( FloatEdit, float )
FLAN_IMPL_EDIT_TYPE( IntEdit, int )
FLAN_IMPL_EDIT_TYPE( Float2Edit, glm::vec2 )
FLAN_IMPL_EDIT_TYPE( Float3Edit, glm::vec3 )
FLAN_IMPL_EDIT_TYPE( Float4Edit, glm::vec4 )
