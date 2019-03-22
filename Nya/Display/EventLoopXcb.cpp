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
#include "EventLoopXcb.h"
#include "DisplaySurfaceXcb.h"

#include <Input/InputReaderXcb.h>

void nya::display::PollSystemEventsImpl( NativeDisplaySurface* surface, InputReader* inputReader )
{
    auto event = xcb_poll_for_event( surface->Connection );

    while( event != nullptr ) {
        switch ( event->response_type & 0x7F ) {
        case XCB_CLIENT_MESSAGE: {
            xcb_client_message_event_t* clientMessage = reinterpret_cast<xcb_client_message_event_t *>( event );

            if( clientMessage->window != surface->WindowInstance ) {
                break;
            }

            if( clientMessage->type == surface->WindowProtocolAtom && clientMessage->data.data32[0] == surface->DeleteAtom ) {
                surface->Flags.ShouldQuit = 1;
            }
            break;
        }

        case XCB_EXPOSE: {
            xcb_client_message_event_t clientMessage;
            clientMessage.response_type = XCB_CLIENT_MESSAGE;
            clientMessage.format = 32;
            clientMessage.window = surface->WindowInstance;
            clientMessage.type = XCB_ATOM_NOTICE;

            xcb_send_event( surface->Connection, 0, surface->WindowInstance, 0, reinterpret_cast< const char * >( &clientMessage ) );
            break;
        }

        case XCB_KEY_RELEASE:
        case XCB_KEY_PRESS:
        case XCB_BUTTON_RELEASE:
        case XCB_BUTTON_PRESS:
        case XCB_MOTION_NOTIFY:
            nya::input::ProcessInputEventImpl( inputReader, event );
            break;

        case XCB_ENTER_NOTIFY:
            XDefineCursor( surface->XorgDisplay, surface->WindowInstance, surface->BlankCursor );
            break;

        case XCB_LEAVE_NOTIFY:
            XUndefineCursor( surface->XorgDisplay, surface->WindowInstance );
            break;

        default:
            break;
        }

        xcb_flush( surface->Connection );
        free( event );

        event = xcb_poll_for_event( surface->Connection );
    }
}
#endif
