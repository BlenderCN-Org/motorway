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

#include <Input/InputLayouts.h>
#include <FileSystem/FileSystemObject.h>

namespace alone
{
    namespace core
    {
#define FLAN_DECLARE_DEFAULT_INPUT( cmd ) static constexpr flan::core::eInputKey cmd[flan::core::eInputLayout::InputLayout_COUNT] =
        FLAN_DECLARE_DEFAULT_INPUT( CAMERA_MOVE_FORWARD )
        {
            flan::core::eInputKey::KEY_W,
            flan::core::eInputKey::KEY_Z,
            flan::core::eInputKey::KEY_W
        };

        FLAN_DECLARE_DEFAULT_INPUT( CAMERA_MOVE_BACKWARD )
        {
            flan::core::eInputKey::KEY_S,
            flan::core::eInputKey::KEY_S,
            flan::core::eInputKey::KEY_S
        };

        FLAN_DECLARE_DEFAULT_INPUT( CAMERA_MOVE_LEFT )
        {
            flan::core::eInputKey::KEY_A,
            flan::core::eInputKey::KEY_Q,
            flan::core::eInputKey::KEY_A
        };

        FLAN_DECLARE_DEFAULT_INPUT( CAMERA_MOVE_RIGHT )
        {
            flan::core::eInputKey::KEY_D,
            flan::core::eInputKey::KEY_D,
            flan::core::eInputKey::KEY_D
        };
        
        FLAN_DECLARE_DEFAULT_INPUT( CAMERA_TAKE_ALTITUDE )
        {
            flan::core::eInputKey::KEY_E,
            flan::core::eInputKey::KEY_E,
            flan::core::eInputKey::KEY_E
        };
        
        FLAN_DECLARE_DEFAULT_INPUT( CAMERA_LOWER_ALTITUDE )
        {
            flan::core::eInputKey:: KEY_Q,
            flan::core::eInputKey::KEY_A,
            flan::core::eInputKey::KEY_Q
        };

        FLAN_DECLARE_DEFAULT_INPUT( PICK_OBJECT )
        {
            flan::core::eInputKey::MOUSE_LEFT_BUTTON,
            flan::core::eInputKey::MOUSE_LEFT_BUTTON,
            flan::core::eInputKey::MOUSE_LEFT_BUTTON
        };

        FLAN_DECLARE_DEFAULT_INPUT( MOVE_CAMERA )
        {
            flan::core::eInputKey::MOUSE_RIGHT_BUTTON,
            flan::core::eInputKey::MOUSE_RIGHT_BUTTON,
            flan::core::eInputKey::MOUSE_RIGHT_BUTTON
        };


        FLAN_DECLARE_DEFAULT_INPUT( DBGUI_MOD1 )
        {
            flan::core::eInputKey::KEY_CONTROL,
            flan::core::eInputKey::KEY_CONTROL,
            flan::core::eInputKey::KEY_CONTROL
        };

        FLAN_DECLARE_DEFAULT_INPUT( DBGUI_CopyNode )
        {
            flan::core::eInputKey::KEY_C,
            flan::core::eInputKey::KEY_C,
            flan::core::eInputKey::KEY_C
        };

        FLAN_DECLARE_DEFAULT_INPUT( DBGUI_PasteNode )
        {
            flan::core::eInputKey::KEY_V,
            flan::core::eInputKey::KEY_V,
            flan::core::eInputKey::KEY_V
        };

        FLAN_DECLARE_DEFAULT_INPUT( DBGUI_Undo )
        {
            flan::core::eInputKey::KEY_Z,
            flan::core::eInputKey::KEY_Z,
            flan::core::eInputKey::KEY_Z
        };

        FLAN_DECLARE_DEFAULT_INPUT( DBGUI_Redo )
        {
            flan::core::eInputKey::KEY_Y,
            flan::core::eInputKey::KEY_Y,
            flan::core::eInputKey::KEY_Y
        };

        FLAN_DECLARE_DEFAULT_INPUT( DBGUI_OpenDevMenu )
        {
            flan::core::eInputKey::KEY_TILDE,
            flan::core::eInputKey::KEY_TILDE,
            flan::core::eInputKey::KEY_TILDE
        };

        FLAN_DECLARE_DEFAULT_INPUT( DBGUI_DeleteNode )
        {
            flan::core::eInputKey::KEY_DELETEFORWARD,
            flan::core::eInputKey::KEY_DELETEFORWARD,
            flan::core::eInputKey::KEY_DELETEFORWARD
        };

        FLAN_DECLARE_DEFAULT_INPUT( DBGUI_Save )
        {
            flan::core::eInputKey::KEY_S,
            flan::core::eInputKey::KEY_S,
            flan::core::eInputKey::KEY_S
        }; 

        FLAN_DECLARE_DEFAULT_INPUT( DBGUI_Open )
        {
            flan::core::eInputKey::KEY_O,
            flan::core::eInputKey::KEY_O,
            flan::core::eInputKey::KEY_O
        };

        FLAN_DECLARE_DEFAULT_INPUT( MOVE_FORWARD )
        {
            flan::core::eInputKey::KEY_W,
            flan::core::eInputKey::KEY_Z,
            flan::core::eInputKey::KEY_W
        };

        FLAN_DECLARE_DEFAULT_INPUT( MOVE_BACKWARD )
        {
            flan::core::eInputKey::KEY_S,
            flan::core::eInputKey::KEY_S,
            flan::core::eInputKey::KEY_S
        };

        FLAN_DECLARE_DEFAULT_INPUT( MOVE_LEFT )
        {
            flan::core::eInputKey::KEY_A,
            flan::core::eInputKey::KEY_Q,
            flan::core::eInputKey::KEY_A
        };

        FLAN_DECLARE_DEFAULT_INPUT( MOVE_RIGHT )
        {
            flan::core::eInputKey::KEY_D,
            flan::core::eInputKey::KEY_D,
            flan::core::eInputKey::KEY_D
        };

        FLAN_DECLARE_DEFAULT_INPUT( SPRINT )
        {
            flan::core::eInputKey::KEY_LEFTSHIFT,
            flan::core::eInputKey::KEY_LEFTSHIFT,
            flan::core::eInputKey::KEY_LEFTSHIFT
        };
#undef FLAN_DECLARE_DEFAULT_INPUT

        static void WriteDefaultInputCfg( FileSystemObject* stream, const flan::core::eInputLayout inputLayout )
        {
            stream->writeString( "Context: \"Game\" {\n" );   
            stream->writeString( "\tMoveForward: { STATE, " + std::string( flan::core::InputKeyToString[MOVE_FORWARD[inputLayout]] ) + " }\n" );
            stream->writeString( "\tMoveLeft: { STATE, " + std::string( flan::core::InputKeyToString[MOVE_LEFT[inputLayout]] ) + " }\n" );
            stream->writeString( "\tMoveBackward: { STATE, " + std::string( flan::core::InputKeyToString[MOVE_BACKWARD[inputLayout]] ) + " }\n" );
            stream->writeString( "\tMoveRight: { STATE, " + std::string( flan::core::InputKeyToString[MOVE_RIGHT[inputLayout]] ) + " }\n" );
            stream->writeString( "\tSprint: { STATE, " + std::string( flan::core::InputKeyToString[SPRINT[inputLayout]] ) + " }\n" );

            stream->writeString( "\tHeadMoveHorizontal: { AXIS, MOUSE_X, 2000.0, -1000.0, 1000.0, -1.0, 1.0 }\n" );
            stream->writeString( "\tHeadMoveVertical: { AXIS, MOUSE_Y, 2000.0, -1000.0, 1000.0, -1.0, 1.0 }\n\n" );

            stream->writeString( "}\n" );
#if FLAN_DEVBUILD
            stream->writeString( "Context: \"Editor\" {\n" );
            stream->writeString( "\tOpenDevMenu: { ACTION, " + std::string( flan::core::InputKeyToString[DBGUI_OpenDevMenu[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraMoveForward: { STATE, " + std::string( flan::core::InputKeyToString[CAMERA_MOVE_FORWARD[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraMoveLeft: { STATE, " + std::string( flan::core::InputKeyToString[CAMERA_MOVE_LEFT[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraMoveBackward: { STATE, " + std::string( flan::core::InputKeyToString[CAMERA_MOVE_BACKWARD[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraMoveRight: { STATE, " + std::string( flan::core::InputKeyToString[CAMERA_MOVE_RIGHT[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraLowerAltitude: { STATE, " + std::string( flan::core::InputKeyToString[CAMERA_LOWER_ALTITUDE[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraTakeAltitude: { STATE, " + std::string( flan::core::InputKeyToString[CAMERA_TAKE_ALTITUDE[inputLayout]] ) + " }\n\n" );

            stream->writeString( "\tCameraMoveHorizontal: { AXIS, MOUSE_X, 2000.0, -1000.0, 1000.0, -1.0, 1.0 }\n" );
            stream->writeString( "\tCameraMoveVertical: { AXIS, MOUSE_Y, 2000.0, -1000.0, 1000.0, -1.0, 1.0 }\n\n" );

            stream->writeString( "}\n" );
            stream->writeString( "Context: \"DebugUI\" {\n" );
            stream->writeString( "\tPickNode: { ACTION, " + std::string( flan::core::InputKeyToString[PICK_OBJECT[inputLayout]] ) + " }\n" );
            stream->writeString( "\tMoveCamera: { STATE, " + std::string( flan::core::InputKeyToString[MOVE_CAMERA[inputLayout]] ) + " }\n" );

            stream->writeString( "\tModifier1: { STATE, " + std::string( flan::core::InputKeyToString[DBGUI_MOD1[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCopyNode: { ACTION, " + std::string( flan::core::InputKeyToString[DBGUI_CopyNode[inputLayout]] ) + " }\n" );
            stream->writeString( "\tPasteNode: { ACTION, " + std::string( flan::core::InputKeyToString[DBGUI_PasteNode[inputLayout]] ) + " }\n" );
            stream->writeString( "\tSaveScene: { ACTION, " + std::string( flan::core::InputKeyToString[DBGUI_Save[inputLayout]] ) + " }\n" );
            stream->writeString( "\tOpenScene: { ACTION, " + std::string( flan::core::InputKeyToString[DBGUI_Open[inputLayout]] ) + " }\n" );
            stream->writeString( "\tUndo: { ACTION, " + std::string( flan::core::InputKeyToString[DBGUI_Undo[inputLayout]] ) + " }\n" );
            stream->writeString( "\tRedo: { ACTION, " + std::string( flan::core::InputKeyToString[DBGUI_Redo[inputLayout]] ) + " }\n" );
            stream->writeString( "\tDeleteNode: { ACTION, " + std::string( flan::core::InputKeyToString[DBGUI_DeleteNode[inputLayout]] ) + " }\n" );
            stream->writeString( "}\n" );
#endif
        }
    }
}
