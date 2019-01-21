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
#include "DisplaySurfaceWin32.h"
#include "EventLoopWin32.h"

LRESULT CALLBACK WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    NativeDisplaySurface* nativeSurface = reinterpret_cast<NativeDisplaySurface*>( GetWindowLongPtr( hwnd, GWLP_USERDATA ) );

    if ( nativeSurface == nullptr ) {
        return DefWindowProc( hwnd, uMsg, wParam, lParam );
    }

    switch ( uMsg ) {
    case WM_DESTROY:
        DestroyWindow( hwnd );
        return 0;

    case WM_CLOSE:
        nativeSurface->Flags.ShouldQuit = 1;
        PostQuitMessage( 0 );
        return 0;

    case WM_SETCURSOR:
    {
        WORD ht = LOWORD( lParam );

        // Detect if the mouse is inside the windows rectangle
        // If it is not (e.g. the mouse is on the window title bar), display the cursor
        if ( HTCLIENT == ht && nativeSurface->Flags.IsMouseVisible ) {
            nativeSurface->Flags.IsMouseVisible = 0;
            ShowCursor( FALSE );
        } else if ( HTCLIENT != ht && nativeSurface->Flags.IsMouseVisible == 0 ) {
            nativeSurface->Flags.IsMouseVisible = 1;
            ShowCursor( TRUE );
        }
    } return 0;

    case WM_ACTIVATE:
    case WM_SETFOCUS:
        nativeSurface->Flags.HasFocus = 1;
        return 0;

    case WA_INACTIVE:
    case WM_KILLFOCUS:
        nativeSurface->Flags.HasFocus = 0;
        nativeSurface->Flags.HasLostFocus = 1;
        return 0;

    default:
        break;
    }

    return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

NativeDisplaySurface* nya::display::CreateDisplaySurfaceImpl( BaseAllocator* allocator, const uint32_t surfaceWidth, const uint32_t surfaceHeight )
{
    NYA_CLOG << "Creating display surface (WIN32)" << std::endl;

    auto instance = GetModuleHandle( NULL );
    auto className = NYA_STRING( "Nya GameEngine Window" );

    HICON hIcon = LoadIcon( instance, MAKEINTRESOURCE( 101 ) );

    const WNDCLASSEX wc = {
        sizeof( WNDCLASSEX ),									// cbSize
        CS_HREDRAW | CS_VREDRAW | CS_OWNDC,						// style
        ::WndProc,												// lpnyaWndProc
        NULL,													// cbClsExtra
        NULL,													// cbWndExtra
        instance,											    // hInstance
        hIcon,			                                        // hIcon
        LoadCursor( 0, IDC_ARROW ),								// hCursor
        static_cast<HBRUSH>( GetStockObject( BLACK_BRUSH ) ),	// hbrBackground
        NULL,													// lpszMenuName
        className,											    // lpszClassName
        NULL 													// hIconSm
    };

    if ( RegisterClassEx( &wc ) == FALSE ) {
        return nullptr;
    }

    DWORD windowExFlags = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, windowFlags = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ( WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_VISIBLE );
    
    int screenWidth = GetSystemMetrics( SM_CXSCREEN ),
        screenHeight = GetSystemMetrics( SM_CYSCREEN );

    auto handle = CreateWindowEx( windowExFlags, className, className, windowFlags, ( screenWidth - surfaceWidth ) / 2, ( screenHeight - surfaceHeight ) / 2, surfaceWidth, surfaceHeight, NULL, NULL, instance, NULL );

    if ( handle == nullptr ) {
        return nullptr;
    }

    SendMessage( handle, WM_SETICON, ICON_SMALL, ( LPARAM )hIcon );
    SendMessage( handle, WM_SETICON, ICON_BIG, ( LPARAM )hIcon );

    auto drawContext = GetDC( handle );

    ShowWindow( handle, SW_SHOWNORMAL );

    UpdateWindow( handle );

    SetForegroundWindow( handle );
    SetFocus( handle );

    RECT rcClient, rcWind;
    POINT ptDiff;
    GetClientRect( handle, &rcClient );
    GetWindowRect( handle, &rcWind );
    ptDiff.x = ( rcWind.right - rcWind.left ) - rcClient.right;
    ptDiff.y = ( rcWind.bottom - rcWind.top ) - rcClient.bottom;

    auto clientWidth = static_cast<int>( surfaceWidth + ptDiff.x );
    auto clientHeight = static_cast<int>( surfaceHeight + ptDiff.y );

    MoveWindow( handle, rcWind.left - ptDiff.x, rcWind.top, clientWidth, clientHeight, TRUE );

    NativeDisplaySurface* displaySurface = nya::core::allocate<NativeDisplaySurface>( allocator );
    displaySurface->ClassName = className;
    displaySurface->DrawContext = drawContext;
    displaySurface->Handle = handle;
    displaySurface->Instance = instance;
    displaySurface->WindowWidth = surfaceWidth;
    displaySurface->WindowHeight = surfaceHeight;
    displaySurface->ClientWidth = clientWidth;
    displaySurface->ClientHeight = clientHeight;

    SetWindowLongPtr( handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( displaySurface ) );

    return displaySurface;
}

void nya::display::SetDisplaySurfaceCaptionImpl( NativeDisplaySurface* surface, const nyaChar_t* caption )
{
    SetWindowText( surface->Handle, caption );
}

void nya::display::ToggleFullscreenImpl( NativeDisplaySurface* surface )
{
    DWORD windowExFlags = WS_EX_APPWINDOW, windowFlags = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    int screenWidth = GetSystemMetrics( SM_CXSCREEN ),
        screenHeight = GetSystemMetrics( SM_CYSCREEN );

    RECT windowRectangle;
    GetWindowRect( surface->Handle, &windowRectangle );

    auto width = windowRectangle.right - windowRectangle.left;
    auto height = windowRectangle.bottom - windowRectangle.top;

    DEVMODE devMode = {};
    devMode.dmSize = sizeof( devMode );
    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmBitsPerPel = 32;
    devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    if ( ChangeDisplaySettings( &devMode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL ) {
        NYA_CERR << "Failed to toggle fullscreen (ChangeDisplaySettings failed)" << std::endl;
    }
}

void nya::display::ToggleBorderlessImpl( NativeDisplaySurface* surface )
{
    RECT windowRectangle;
    GetWindowRect( surface->Handle, &windowRectangle );

    auto width = windowRectangle.right - windowRectangle.left;
    auto height = windowRectangle.bottom - windowRectangle.top;

    SetWindowLongPtr( surface->Handle, GWL_STYLE, WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE );
    MoveWindow( surface->Handle, 0, 0, width, height, TRUE );
}

bool nya::display::GetShouldQuitFlagImpl( NativeDisplaySurface* surface )
{
    return ( surface->Flags.ShouldQuit == 1 );
}

void nya::display::SetMousePositionImpl( NativeDisplaySurface* surface, const float surfaceNormalizedX, const float surfaceNormalizedY )
{
    // Don't trap mouse if the user has alt-tabbed the application
    if ( surface->Flags.HasFocus == 0 ) {
        return;
    }

    POINT screenPoint = { 0 };
    screenPoint.x = static_cast< LONG >( surface->ClientWidth * surfaceNormalizedX );
    screenPoint.y = static_cast< LONG >( surface->ClientHeight * surfaceNormalizedY );

    ClientToScreen( surface->Handle, &screenPoint );

    SetCursorPos( screenPoint.x, screenPoint.y );
}
#endif
