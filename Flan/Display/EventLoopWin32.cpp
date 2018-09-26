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

#if FLAN_WIN
#include "EventLoopWin32.h"
#include "DisplaySurfaceWin32.h"

#include <Input/InputReader.h>
#include <Input/InputReaderWin32.h>
#include <Input/InputKeys.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

void flan::core::PollSystemEventsImpl( NativeDisplaySurface* surface, InputReader* inputReader )
{
    MSG msg = { 0 };

    FLAN_IMPORT_VAR_PTR( IsDevMenuVisible, bool )

    while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
#if FLAN_D3D11
        if ( *IsDevMenuVisible 
            && ImGui_ImplWin32_WndProcHandler( msg.hwnd, msg.message, msg.wParam, msg.lParam ) )
            continue;
#endif

        if ( msg.message == WM_INPUT ) {
            ProcessInputEventImpl( inputReader, msg.hwnd, msg.lParam, msg.time );
        } else if ( msg.message == WM_QUIT ) {
            surface->Flags.ShouldQuit = 1;
        }

        //else if ( msg.message == WM_CHAR ) {
        //    // Convert scancode to virtual key (WindowsKeys map is based on vk codes)
        //    auto scancode = ( reinterpret_cast<unsigned char*>( &msg.lParam ) )[2];
        //    auto virtualKey = MapVirtualKey( scancode, MAPVK_VSC_TO_VK );

        //    inputReader->pushKeyStroke( WindowsKeys[virtualKey] );
        //}

        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}
#endif
