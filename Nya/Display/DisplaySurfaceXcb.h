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

#if NYA_UNIX
#include <xcb/xcb.h>
#include <X11/Xlib.h>

struct NativeDisplaySurface
{
    xcb_connection_t*   Connection;
    xcb_drawable_t      WindowInstance;

    xcb_atom_t WindowProtocolAtom;
    xcb_atom_t DeleteAtom;

    uint32_t    Width;
    uint32_t    Height;

    Cursor      BlankCursor;
    Display*    XorgDisplay;

    struct
    {
        uint32_t        ShouldQuit : 1;
    } Flags;

    ~NativeDisplaySurface()
    {       
        xcb_destroy_window( Connection, WindowInstance );
    }
};

namespace nya
{
    namespace display
    {
        NativeDisplaySurface*   CreateDisplaySurfaceImpl( const uint32_t surfaceWidth, const uint32_t surfaceHeight );
        void                    SetDisplaySurfaceCaptionImpl( NativeDisplaySurface* surface, const nyaString_t& caption = NYA_STRING( "Flan GameEngine" ) );
        void                    ToggleFullscreenImpl( NativeDisplaySurface* surface );
        void                    ToggleBorderlessImpl( NativeDisplaySurface* surface );
        bool                    GetShouldQuitFlagImpl( NativeDisplaySurface* surface );
        void                    SetMousePositionImpl(  NativeDisplaySurface* surface, const float surfaceNormalizedX, const float surfaceNormalizedY );
    }
}
#endif
