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
#include <imgui/imgui_internal.h>

static bool IsItemActiveLastFrame()
{
    ImGuiContext& g = *ImGui::GetCurrentContext();
    if ( g.ActiveIdPreviousFrame )
        return g.ActiveIdPreviousFrame == g.CurrentWindow->DC.LastItemId;
    return false;
}

#define FLAN_IMGUI_UNDO_IMPL( transactionHandler, variable, cmdType )\
    static decltype( variable ) variable##_Backup = {}; \
    if ( ImGui::IsItemActive() && !IsItemActiveLastFrame() ) {\
        variable##_Backup = variable;\
    }\
    if ( ImGui::IsItemDeactivatedAfterEdit() ) {\
        transactionHandler->commit<cmdType>( &variable, variable, variable##_Backup );\
    }

#define FLAN_IMGUI_UNDO_IMPL_PTR( transactionHandler, variable, cmdType )\
    static decltype( variable ) variable##_Backup = {}; \
    if ( ImGui::IsItemActive() && !IsItemActiveLastFrame() ) {\
        variable##_Backup = variable;\
    }\
    if ( ImGui::IsItemDeactivatedAfterEdit() ) {\
        transactionHandler->commit<cmdType>( variable, *variable, variable##_Backup );\
    }

#define FLAN_IMGUI_DRAG_FLOAT( transactionHandler, variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat( #variable, &variable, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL( transactionHandler, variable, FloatEditCommand )

#define FLAN_IMGUI_DRAG_FLOAT_PTR( transactionHandler, variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat( #variable, variable, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL_PTR( variable, FloatEditCommand )

#define FLAN_IMGUI_DRAG_INT( transactionHandler, variable, step, rangeMin, rangeMax )\
    ImGui::DragInt( #variable, &variable, step, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL( transactionHandler, variable, IntEditCommand )

#define FLAN_IMGUI_DRAG_FLOAT2( transactionHandler, variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat2( #variable, &variable, step, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL( transactionHandler, variable, Float2EditCommand )

#define FLAN_IMGUI_DRAG_FLOAT3( transactionHandler, variable, step, rangeMin, rangeMax )\
    ImGui::DragFloat3( #variable, (float*)&variable, step, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL( transactionHandler, variable, Float3EditCommand )

#define FLAN_IMGUI_INPUT_FLOAT3( transactionHandler, variable )\
    ImGui::InputFloat3( #variable, (float*)&variable );\
    FLAN_IMGUI_UNDO_IMPL( transactionHandler, variable, Float3EditCommand )

#define FLAN_IMGUI_SLIDER_INT( transactionHandler, variable, rangeMin, rangeMax )\
    ImGui::SliderInt( #variable, &variable, rangeMin, rangeMax );\
    FLAN_IMGUI_UNDO_IMPL( transactionHandler, variable, IntEditCommand )

#define FLAN_IMGUI_CHECKBOX( transactionHandler, variable, onChecked )\
    if ( ImGui::Checkbox( #variable, &variable ) ) {\
        onChecked\
    }\
    FLAN_IMGUI_UNDO_IMPL( transactionHandler, variable, BoolEditCommand )

#define FLAN_IMGUI_CHECKBOX_PTR( transactionHandler, variable, onChecked )\
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
FLAN_IMPL_EDIT_TYPE( Float2Edit, nyaVec2f )
FLAN_IMPL_EDIT_TYPE( Float3Edit, nyaVec3f )
FLAN_IMPL_EDIT_TYPE( Float4Edit, nyaVec4f )
