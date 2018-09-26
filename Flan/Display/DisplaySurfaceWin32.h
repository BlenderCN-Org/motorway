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

#if FLAN_WIN

struct NativeDisplaySurface
{
    HINSTANCE			Instance;
    HWND				Handle;
    LPCWSTR				ClassName;
    HDC					DrawContext;
    uint32_t            ClientWidth;
    uint32_t            ClientHeight;

    struct
    {
        uint32_t        ShouldQuit : 1;
        uint32_t        IsMouseVisible : 1;
        uint32_t        HasFocus : 1;
        uint32_t        HasLostFocus : 1;
    } Flags;

    ~NativeDisplaySurface()
    {
        ShowWindow( Handle, SW_HIDE );
        ShowCursor( TRUE );
        CloseWindow( Handle );

        UnregisterClass( ClassName, Instance );

        DeleteObject( DrawContext );
        DeleteObject( Instance );
        DestroyWindow( Handle );

        ClientWidth = 0;
        ClientHeight = 0;
    }
};

namespace flan
{
    namespace core
    {
        NativeDisplaySurface*   CreateDisplaySurfaceImpl( const uint32_t surfaceWidth, const uint32_t surfaceHeight );
        void                    SetDisplaySurfaceCaptionImpl( NativeDisplaySurface* surface, const fnString_t& caption = FLAN_STRING( "Flan GameEngine" ) );
        void                    ToggleFullscreenImpl( NativeDisplaySurface* surface );
        void                    ToggleBorderlessImpl( NativeDisplaySurface* surface );
        bool                    GetShouldQuitFlagImpl( NativeDisplaySurface* surface );
        void                    SetMousePositionImpl( NativeDisplaySurface* surface, const float surfaceNormalizedX, const float surfaceNormalizedY );
    }
}
#endif
