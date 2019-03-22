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

#ifdef NYA_UNIX
#include "InputReaderXcb.h"

#include "InputReader.h"

#include "InputAxis.h"

#include <xcb/xcb_keysyms.h>

static xcb_key_symbols_t* KEY_SYMBOLS = nullptr;

static double previousX = 0.0;
static double previousY = 0.0;

using namespace nya::input;

void nya::input::CreateInputReaderImpl( const nya::input::eInputLayout& activeInputLayout )
{
    auto connection = xcb_connect( nullptr, nullptr );
    KEY_SYMBOLS = xcb_key_symbols_alloc( connection );
}

void nya::input::ProcessInputEventImpl( InputReader* inputReader, const xcb_generic_event_t* event )
{
    const auto responseType = ( event->response_type & 0x7F );

    switch( responseType ) {
    case XCB_MOTION_NOTIFY: {
        const xcb_motion_notify_event_t* motionNotifyEvent = reinterpret_cast<const xcb_motion_notify_event_t*>( event );

        inputReader->pushAxisEvent( { eInputAxis::MOUSE_X, 0, static_cast<double>( motionNotifyEvent->event_x - previousX ) } );
        inputReader->pushAxisEvent( { eInputAxis::MOUSE_Y, 0, static_cast<double>( motionNotifyEvent->event_y - previousY ) } );

        inputReader->setAbsoluteAxisValue( eInputAxis::MOUSE_X, motionNotifyEvent->event_x );
        inputReader->setAbsoluteAxisValue( eInputAxis::MOUSE_Y, motionNotifyEvent->event_y );

        previousX = motionNotifyEvent->event_x;
        previousY = motionNotifyEvent->event_y;

        break;
    }

    case XCB_KEY_RELEASE:{
        const xcb_key_release_event_t* keyReleaseEvent = reinterpret_cast<const xcb_key_release_event_t*>( event );
        inputReader->pushKeyEvent( { nya::input::UnixKeys[xcb_key_symbols_get_keysym( KEY_SYMBOLS, keyReleaseEvent->detail, 0 )], 0 } );
        break;
    }

    case XCB_KEY_PRESS: {
        const xcb_key_press_event_t* keyPressEvent = reinterpret_cast<const xcb_key_press_event_t*>( event );
        inputReader->pushKeyEvent( { nya::input::UnixKeys[xcb_key_symbols_get_keysym( KEY_SYMBOLS, keyPressEvent->detail, 0 )], 1 } );
        break;
    }

    case XCB_BUTTON_PRESS: {
        const xcb_button_press_event_t* pressEvent = reinterpret_cast<const xcb_button_press_event_t*>( event );

        switch ( pressEvent->detail ) {
        case XCB_BUTTON_INDEX_1:
            inputReader->pushKeyEvent( { eInputKey::MOUSE_LEFT_BUTTON, true } );
            break;
        case XCB_BUTTON_INDEX_2:
            inputReader->pushKeyEvent( { eInputKey::MOUSE_RIGHT_BUTTON, true } );
            break;
        case XCB_BUTTON_INDEX_3:
            inputReader->pushKeyEvent( { eInputKey::MOUSE_MIDDLE_BUTTON, true } );
            break;
        default:
            break;
        }
        break;
    }

    case XCB_BUTTON_RELEASE: {
        const xcb_button_release_event_t* releaseEvent = reinterpret_cast<const xcb_button_release_event_t*>( event );

        switch ( releaseEvent->detail ) {
        case XCB_BUTTON_INDEX_1:
            inputReader->pushKeyEvent( { eInputKey::MOUSE_LEFT_BUTTON, false } );
            break;
        case XCB_BUTTON_INDEX_2:
            inputReader->pushKeyEvent( { eInputKey::MOUSE_RIGHT_BUTTON, false } );
            break;
        case XCB_BUTTON_INDEX_3:
            inputReader->pushKeyEvent( { eInputKey::MOUSE_MIDDLE_BUTTON, false } );
            break;
        default:
            break;
        }
        break;
    }

    default:
        break;
    };
}
#endif
