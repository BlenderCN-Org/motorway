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
#include "InputReader.h"

#include "InputMapper.h"
#include "InputAxis.h"

#if NYA_WIN
#include "InputReaderWin32.h"
#elif NYA_UNIX
#include "InputReaderXcb.h"
#endif

using namespace nya::input;

InputReader::InputReader()
    : activeInputLayout( INPUT_LAYOUT_QWERTY )
    , textInput( 1024 )
    , absoluteAxisValues{ 0.0 }
    , buttonMap{ false }
{

}

InputReader::~InputReader()
{
    memset( buttonMap, 0, sizeof( bool ) * KEYBOARD_KEY_COUNT );
}

void InputReader::create()
{
    CreateInputReaderImpl( activeInputLayout );

    NYA_CLOG << "Active keyboard layout: " << activeInputLayout << std::endl;
}

void InputReader::onFrame( InputMapper* inputMapper )
{
    // Process and dispatch captured events to an input mapper
    while ( !inputKeyEventQueue.empty() ) {
        auto& inputKeyEvent = inputKeyEventQueue.front();
        auto wasPreviouslyDown = buttonMap[inputKeyEvent.InputKey];

        inputMapper->setRawButtonState( inputKeyEvent.InputKey, inputKeyEvent.IsDown, wasPreviouslyDown );

        // Update Previous Key State
        buttonMap[inputKeyEvent.InputKey] = inputKeyEvent.IsDown;

        inputKeyEventQueue.pop();
    }

    // Do the same for axis updates
    while ( !inputAxisEventQueue.empty() ) {
        auto& inputAxisEvent = inputAxisEventQueue.front();

        inputMapper->setRawAxisValue( inputAxisEvent.InputAxis, inputAxisEvent.RawAxisValue );

        inputAxisEventQueue.pop();
    }
}

nya::input::eInputLayout InputReader::getActiveInputLayout() const
{
    return activeInputLayout;
}

void InputReader::pushKeyEvent( InputReader::KeyEvent&& keyEvent )
{
    inputKeyEventQueue.push( std::move( keyEvent ) );
}

void InputReader::pushAxisEvent( InputReader::AxisEvent&& axisEvent )
{
    inputAxisEventQueue.push( std::move( axisEvent ) );
}

void InputReader::pushKeyStroke( const eInputKey keyStroke )
{
    textInput.push_back( keyStroke );
}

std::vector<nya::input::eInputKey> InputReader::getAndFlushKeyStrokes()
{
    auto keyStrokes = textInput;
    textInput.clear();
    return keyStrokes;
}

void InputReader::setAbsoluteAxisValue( const nya::input::eInputAxis axis, const double absoluteValue )
{
    absoluteAxisValues[axis] = absoluteValue;
}

double InputReader::getAbsoluteAxisValue( const nya::input::eInputAxis axis )
{
    return absoluteAxisValues[axis];
}
