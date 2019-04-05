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

#include <Input/InputKeys.h>
#include <Input/InputLayouts.h>

#include <FileSystem/FileSystemObject.h>

namespace nya
{
    namespace core
    {
#define NYA_DECLARE_DEFAULT_INPUT( cmd ) static constexpr nya::input::eInputKey cmd[nya::input::eInputLayout::InputLayout_COUNT] =
        NYA_DECLARE_DEFAULT_INPUT( CAMERA_MOVE_FORWARD )
        {
            nya::input::eInputKey::KEY_W,
            nya::input::eInputKey::KEY_Z,
            nya::input::eInputKey::KEY_W
        };

        NYA_DECLARE_DEFAULT_INPUT( CAMERA_MOVE_BACKWARD )
        {
            nya::input::eInputKey::KEY_S,
            nya::input::eInputKey::KEY_S,
            nya::input::eInputKey::KEY_S
        };

        NYA_DECLARE_DEFAULT_INPUT( CAMERA_MOVE_LEFT )
        {
            nya::input::eInputKey::KEY_A,
            nya::input::eInputKey::KEY_Q,
            nya::input::eInputKey::KEY_A
        };

        NYA_DECLARE_DEFAULT_INPUT( CAMERA_MOVE_RIGHT )
        {
            nya::input::eInputKey::KEY_D,
            nya::input::eInputKey::KEY_D,
            nya::input::eInputKey::KEY_D
        };
        
        NYA_DECLARE_DEFAULT_INPUT( CAMERA_TAKE_ALTITUDE )
        {
            nya::input::eInputKey::KEY_E,
            nya::input::eInputKey::KEY_E,
            nya::input::eInputKey::KEY_E
        };
        
        NYA_DECLARE_DEFAULT_INPUT( CAMERA_LOWER_ALTITUDE )
        {
            nya::input::eInputKey:: KEY_Q,
            nya::input::eInputKey::KEY_A,
            nya::input::eInputKey::KEY_Q
        };

        NYA_DECLARE_DEFAULT_INPUT( PICK_OBJECT )
        {
            nya::input::eInputKey::MOUSE_LEFT_BUTTON,
            nya::input::eInputKey::MOUSE_LEFT_BUTTON,
            nya::input::eInputKey::MOUSE_LEFT_BUTTON
        };

        NYA_DECLARE_DEFAULT_INPUT( MOVE_CAMERA )
        {
            nya::input::eInputKey::MOUSE_RIGHT_BUTTON,
            nya::input::eInputKey::MOUSE_RIGHT_BUTTON,
            nya::input::eInputKey::MOUSE_RIGHT_BUTTON
        };


        NYA_DECLARE_DEFAULT_INPUT( DBGUI_MOD1 )
        {
            nya::input::eInputKey::KEY_CONTROL,
            nya::input::eInputKey::KEY_CONTROL,
            nya::input::eInputKey::KEY_CONTROL
        };

        NYA_DECLARE_DEFAULT_INPUT( DBGUI_CopyNode )
        {
            nya::input::eInputKey::KEY_C,
            nya::input::eInputKey::KEY_C,
            nya::input::eInputKey::KEY_C
        };

        NYA_DECLARE_DEFAULT_INPUT( DBGUI_PasteNode )
        {
            nya::input::eInputKey::KEY_V,
            nya::input::eInputKey::KEY_V,
            nya::input::eInputKey::KEY_V
        };

        NYA_DECLARE_DEFAULT_INPUT( DBGUI_Undo )
        {
            nya::input::eInputKey::KEY_Z,
            nya::input::eInputKey::KEY_Z,
            nya::input::eInputKey::KEY_Z
        };

        NYA_DECLARE_DEFAULT_INPUT( DBGUI_Redo )
        {
            nya::input::eInputKey::KEY_Y,
            nya::input::eInputKey::KEY_Y,
            nya::input::eInputKey::KEY_Y
        };

        NYA_DECLARE_DEFAULT_INPUT( DBGUI_OpenDevMenu )
        {
            nya::input::eInputKey::KEY_TILDE,
            nya::input::eInputKey::KEY_TILDE,
            nya::input::eInputKey::KEY_TILDE
        };

        NYA_DECLARE_DEFAULT_INPUT( DBGUI_DeleteNode )
        {
            nya::input::eInputKey::KEY_DELETEFORWARD,
            nya::input::eInputKey::KEY_DELETEFORWARD,
            nya::input::eInputKey::KEY_DELETEFORWARD
        };

        NYA_DECLARE_DEFAULT_INPUT( DBGUI_Save )
        {
            nya::input::eInputKey::KEY_S,
            nya::input::eInputKey::KEY_S,
            nya::input::eInputKey::KEY_S
        }; 

        NYA_DECLARE_DEFAULT_INPUT( DBGUI_Open )
        {
            nya::input::eInputKey::KEY_O,
            nya::input::eInputKey::KEY_O,
            nya::input::eInputKey::KEY_O
        };

        NYA_DECLARE_DEFAULT_INPUT( MOVE_FORWARD )
        {
            nya::input::eInputKey::KEY_W,
            nya::input::eInputKey::KEY_Z,
            nya::input::eInputKey::KEY_W
        };

        NYA_DECLARE_DEFAULT_INPUT( MOVE_BACKWARD )
        {
            nya::input::eInputKey::KEY_S,
            nya::input::eInputKey::KEY_S,
            nya::input::eInputKey::KEY_S
        };

        NYA_DECLARE_DEFAULT_INPUT( MOVE_LEFT )
        {
            nya::input::eInputKey::KEY_A,
            nya::input::eInputKey::KEY_Q,
            nya::input::eInputKey::KEY_A
        };

        NYA_DECLARE_DEFAULT_INPUT( MOVE_RIGHT )
        {
            nya::input::eInputKey::KEY_D,
            nya::input::eInputKey::KEY_D,
            nya::input::eInputKey::KEY_D
        };

        NYA_DECLARE_DEFAULT_INPUT( SPRINT )
        {
            nya::input::eInputKey::KEY_LEFTSHIFT,
            nya::input::eInputKey::KEY_LEFTSHIFT,
            nya::input::eInputKey::KEY_LEFTSHIFT
        };
#undef NYA_DECLARE_DEFAULT_INPUT

        static void WriteDefaultInputCfg( FileSystemObject* stream, const nya::input::eInputLayout inputLayout )
        {
            stream->writeString( "Context: \"Game\" {\n" );   
            stream->writeString( "\tMoveForward: { STATE, " + std::string( nya::input::InputKeyToString[MOVE_FORWARD[inputLayout]] ) + " }\n" );
            stream->writeString( "\tMoveLeft: { STATE, " + std::string( nya::input::InputKeyToString[MOVE_LEFT[inputLayout]] ) + " }\n" );
            stream->writeString( "\tMoveBackward: { STATE, " + std::string( nya::input::InputKeyToString[MOVE_BACKWARD[inputLayout]] ) + " }\n" );
            stream->writeString( "\tMoveRight: { STATE, " + std::string( nya::input::InputKeyToString[MOVE_RIGHT[inputLayout]] ) + " }\n" );
            stream->writeString( "\tSprint: { STATE, " + std::string( nya::input::InputKeyToString[SPRINT[inputLayout]] ) + " }\n" );

            stream->writeString( "\tHeadMoveHorizontal: { AXIS, MOUSE_X, 2000.0, -1000.0, 1000.0, -1.0, 1.0 }\n" );
            stream->writeString( "\tHeadMoveVertical: { AXIS, MOUSE_Y, 2000.0, -1000.0, 1000.0, -1.0, 1.0 }\n" );

            stream->writeString( "}\n" );
#if NYA_DEVBUILD
            stream->writeString( "Context: \"Editor\" {\n" );
            stream->writeString( "\tOpenDevMenu: { ACTION, " + std::string( nya::input::InputKeyToString[DBGUI_OpenDevMenu[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraMoveForward: { STATE, " + std::string( nya::input::InputKeyToString[CAMERA_MOVE_FORWARD[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraMoveLeft: { STATE, " + std::string( nya::input::InputKeyToString[CAMERA_MOVE_LEFT[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraMoveBackward: { STATE, " + std::string( nya::input::InputKeyToString[CAMERA_MOVE_BACKWARD[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraMoveRight: { STATE, " + std::string( nya::input::InputKeyToString[CAMERA_MOVE_RIGHT[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraLowerAltitude: { STATE, " + std::string( nya::input::InputKeyToString[CAMERA_LOWER_ALTITUDE[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCameraTakeAltitude: { STATE, " + std::string( nya::input::InputKeyToString[CAMERA_TAKE_ALTITUDE[inputLayout]] ) + " }\n\n" );

            stream->writeString( "\tCameraMoveHorizontal: { AXIS, MOUSE_X, 2000.0, -1000.0, 1000.0, -1.0, 1.0 }\n" );
            stream->writeString( "\tCameraMoveVertical: { AXIS, MOUSE_Y, 2000.0, -1000.0, 1000.0, -1.0, 1.0 }\n" );

            stream->writeString( "}\n" );
            stream->writeString( "Context: \"DebugUI\" {\n" );
            stream->writeString( "\tMouseClick: { STATE, " + std::string( nya::input::InputKeyToString[PICK_OBJECT[inputLayout]] ) + " }\n" );
            stream->writeString( "\tMoveCamera: { STATE, " + std::string( nya::input::InputKeyToString[MOVE_CAMERA[inputLayout]] ) + " }\n" );

            stream->writeString( "\tModifier1: { STATE, " + std::string( nya::input::InputKeyToString[DBGUI_MOD1[inputLayout]] ) + " }\n" );
            stream->writeString( "\tCopyNode: { ACTION, " + std::string( nya::input::InputKeyToString[DBGUI_CopyNode[inputLayout]] ) + " }\n" );
            stream->writeString( "\tPasteNode: { ACTION, " + std::string( nya::input::InputKeyToString[DBGUI_PasteNode[inputLayout]] ) + " }\n" );
            stream->writeString( "\tSaveScene: { ACTION, " + std::string( nya::input::InputKeyToString[DBGUI_Save[inputLayout]] ) + " }\n" );
            stream->writeString( "\tOpenScene: { ACTION, " + std::string( nya::input::InputKeyToString[DBGUI_Open[inputLayout]] ) + " }\n" );
            stream->writeString( "\tUndo: { ACTION, " + std::string( nya::input::InputKeyToString[DBGUI_Undo[inputLayout]] ) + " }\n" );
            stream->writeString( "\tRedo: { ACTION, " + std::string( nya::input::InputKeyToString[DBGUI_Redo[inputLayout]] ) + " }\n" );
            stream->writeString( "\tDeleteNode: { ACTION, " + std::string( nya::input::InputKeyToString[DBGUI_DeleteNode[inputLayout]] ) + " }\n" );
            stream->writeString( "}\n" );
#endif
        }
    }
}
