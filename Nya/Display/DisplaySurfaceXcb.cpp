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

#if NYA_UNIX
#include "DisplaySurfaceXcb.h"

#include <stb_image.h>
#include <vector>

inline xcb_intern_atom_cookie_t GetCookieForAtom( xcb_connection_t* connection, const nyaString_t& stateName )
{
    return xcb_intern_atom( connection, 0, stateName.size(), stateName.c_str() );
}

xcb_atom_t GetReplyAtomFromCookie( xcb_connection_t* connection, xcb_intern_atom_cookie_t cookie )
{
    xcb_generic_error_t* error = nullptr;
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply( connection, cookie, &error );
    if ( error != nullptr ) {
        NYA_CERR << "Internal failure! (error code:" << error->error_code << ")" << std::endl;
    }

    return reply->atom;
}

NativeDisplaySurface* nya::display::CreateDisplaySurfaceImpl( const uint32_t surfaceWidth, const uint32_t surfaceHeight )
{
    NYA_CLOG << "Creating display surface (XCB)" << std::endl;

    xcb_connection_t* connection = xcb_connect( nullptr, nullptr );
    if ( connection == nullptr ) {
        NYA_CERR << "Failed to connect to Xorg server (xcb_connect returned nullptr)" << std::endl;
        return nullptr;
    }

    NYA_CLOG << "\tConnected to X11 server" << std::endl;

    xcb_screen_t* screen = xcb_setup_roots_iterator( xcb_get_setup( connection ) ).data;

    // Create native resource object
    NativeDisplaySurface* surf = new NativeDisplaySurface();
    surf->Connection = connection;
    surf->WindowInstance = xcb_generate_id( surf->Connection );

    surf->Width = surfaceWidth;
    surf->Height = surfaceHeight;

    constexpr uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    uint32_t values[2] = {
        screen->black_pixel,
        ( XCB_EVENT_MASK_KEY_PRESS |
          XCB_EVENT_MASK_KEY_RELEASE |
          XCB_EVENT_MASK_EXPOSURE |
          XCB_EVENT_MASK_STRUCTURE_NOTIFY |
          XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
          XCB_EVENT_MASK_POINTER_MOTION |
          XCB_EVENT_MASK_BUTTON_PRESS |
          XCB_EVENT_MASK_BUTTON_RELEASE |
          XCB_EVENT_MASK_RESIZE_REDIRECT ),
    };

    xcb_create_window( surf->Connection,
                       XCB_COPY_FROM_PARENT,
                       surf->WindowInstance,
                       screen->root,
                       surfaceWidth * 0.5f, surfaceHeight * 0.5f,
                       surfaceWidth, surfaceHeight,
                       10,
                       XCB_WINDOW_CLASS_INPUT_OUTPUT,
                       screen->root_visual,
                       mask,
                       values );

    // Map the window on the screen
    xcb_map_window( surf->Connection, surf->WindowInstance );
    xcb_flush( surf->Connection );

    NYA_CLOG << "\tCreated and mapped window to screen successfully!" << std::endl;

    // Register WM_DELETE atom
    surf->WindowProtocolAtom = GetReplyAtomFromCookie( surf->Connection, GetCookieForAtom( surf->Connection, "WM_PROTOCOLS" ) );
    surf->DeleteAtom  = GetReplyAtomFromCookie( surf->Connection, GetCookieForAtom( surf->Connection, "WM_DELETE_WINDOW" ) );

    xcb_change_property( surf->Connection, XCB_PROP_MODE_REPLACE, surf->WindowInstance, surf->WindowProtocolAtom, XCB_ATOM_ATOM, 32, 1, &surf->DeleteAtom );

    // Set default default icon
    int w, h, comp;
    auto image = stbi_load( "nya_icon.png", &w, &h, &comp, STBI_rgb_alpha );
    if ( image != nullptr ) {
        const int texelCount = w * h;
        const int iconBufferLength = texelCount + 2; // First two integers contain icon width and height

        std::vector<unsigned int> iconBuffer( iconBufferLength );
        iconBuffer[0] = w;
        iconBuffer[1] = h;

        memcpy( &iconBuffer[2], image, texelCount * comp * sizeof( unsigned char ) );

        stbi_image_free( image );

        auto winIconAtom = GetReplyAtomFromCookie( surf->Connection, GetCookieForAtom( surf->Connection, "_NET_WM_ICON" ) );

        xcb_change_property( surf->Connection,
                             XCB_PROP_MODE_REPLACE,
                             surf->WindowInstance,
                             winIconAtom,
                             XCB_ATOM_CARDINAL,
                             32,
                             iconBufferLength,
                             iconBuffer.data() );
    }

    // Create blank cursor
    XColor dummy = {};
    char BLANK_CURSOR_TEXELS[1] = { 0 };

    Display* tmpDisp = XOpenDisplay( 0 );

    Pixmap blank = XCreateBitmapFromData( tmpDisp, surf->WindowInstance, BLANK_CURSOR_TEXELS, 1, 1 );

    auto blankCursor = XCreatePixmapCursor( tmpDisp, blank, blank, &dummy, &dummy, 0, 0 ) ;
    XFreePixmap( tmpDisp, blank );

    XDefineCursor( tmpDisp, surf->WindowInstance, blankCursor );

    surf->BlankCursor = blankCursor;
    surf->XorgDisplay = tmpDisp;

    return surf;
}

void nya::display::SetDisplaySurfaceCaptionImpl( NativeDisplaySurface* surface, const nyaString_t& caption )
{
    xcb_change_property( surface->Connection, XCB_PROP_MODE_REPLACE, surface->WindowInstance, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, caption.size(), caption.c_str() );
}

void nya::display::ToggleFullscreenImpl( NativeDisplaySurface* surface )
{
    xcb_intern_atom_cookie_t wm_state_ck = GetCookieForAtom( surface->Connection, "_NET_WM_STATE" );
    xcb_intern_atom_cookie_t wm_state_fs_ck = GetCookieForAtom( surface->Connection, "_NET_WM_STATE_FULLSCREEN" );

    constexpr int32_t NET_WM_STATE_TOGGLE = 2;

    xcb_client_message_event_t event = { 0 };
    event.response_type = XCB_CLIENT_MESSAGE;
    event.type = GetReplyAtomFromCookie( surface->Connection, wm_state_ck );
    event.format = 32;
    event.window = surface->WindowInstance;
    event.data.data32[0] = NET_WM_STATE_TOGGLE;
    event.data.data32[1] = GetReplyAtomFromCookie( surface->Connection, wm_state_fs_ck );
    event.data.data32[2] = XCB_ATOM_NONE;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

    constexpr uint32_t EVENT_MASK = ( XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY );

    xcb_send_event( surface->Connection, 1, surface->WindowInstance, EVENT_MASK, (const char*)&event );
}

void nya::display::ToggleBorderlessImpl( NativeDisplaySurface* surface )
{

}

bool nya::display::GetShouldQuitFlagImpl( NativeDisplaySurface* surface )
{
    return ( surface->Flags.ShouldQuit == 1 );
}

void nya::display::SetMousePositionImpl(  NativeDisplaySurface* surface, const float surfaceNormalizedX, const float surfaceNormalizedY )
{
    xcb_warp_pointer( surface->Connection, surface->WindowInstance, surface->WindowInstance, 0, 0, surface->Width, surface->Height,
                      surface->Width * surfaceNormalizedX, surface->Height * surfaceNormalizedY );
}
#endif
