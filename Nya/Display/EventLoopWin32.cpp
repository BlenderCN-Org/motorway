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

#if NYA_WIN
#include "EventLoopWin32.h"
#include "DisplaySurfaceWin32.h"

#include <Input/InputReaderWin32.h>

void nya::display::PollSystemEventsImpl( NativeDisplaySurface* surface, InputReader* inputReader )
{
    MSG msg = { 0 };

    while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
        if ( msg.message == WM_INPUT ) {
            nya::input::ProcessInputEventImpl( inputReader, msg.hwnd, msg.lParam, msg.time );
        } else if ( msg.message == WM_QUIT ) {
            surface->Flags.ShouldQuit = 1;
        }

        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}
#endif
