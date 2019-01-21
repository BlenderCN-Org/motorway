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

namespace nya
{
    namespace input
    {
        enum eInputLayout;
        enum eInputKey;
        enum eInputAxis;
    }
}

#include "InputContext.h"
#include "MappedInput.h"

#include <queue>

class InputMapper;
struct NativeWindow;

using fnKeyStrokes_t = std::vector<nya::input::eInputKey>;

class InputReader
{
public:
    struct KeyEvent {
        nya::input::eInputKey   InputKey;
        int32_t                 IsDown; // Makes the struct 8 bytes aligned
    };

    struct AxisEvent {
        nya::input::eInputAxis  InputAxis;
        uint32_t                __PADDING__;
        double                  RawAxisValue;
    };

public:
                                InputReader();
                                InputReader( InputReader& ) = delete;
                                ~InputReader();

    void                        create();
    void                        onFrame( InputMapper* inputMapper );
    nya::input::eInputLayout    getActiveInputLayout() const;

    void                        pushKeyEvent( InputReader::KeyEvent&& keyEvent );
    void                        pushAxisEvent( InputReader::AxisEvent&& axisEvent );

    void                        pushKeyStroke( const nya::input::eInputKey keyStroke );
    fnKeyStrokes_t              getAndFlushKeyStrokes();

    void                        setAbsoluteAxisValue( const nya::input::eInputAxis axis, const double absoluteValue );
    double                      getAbsoluteAxisValue( const nya::input::eInputAxis axis );

private:
    nya::input::eInputLayout        activeInputLayout;
    std::queue<KeyEvent>            inputKeyEventQueue;
    std::queue<AxisEvent>           inputAxisEventQueue;

    fnKeyStrokes_t                  textInput;

    double                          absoluteAxisValues[nya::input::eInputAxis::InputAxis_COUNT];
    bool                            buttonMap[nya::input::KEYBOARD_KEY_COUNT];
};
