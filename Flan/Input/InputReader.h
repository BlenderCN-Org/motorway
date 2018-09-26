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

#include "InputLayouts.h"
#include "InputKeys.h"
#include "InputAxis.h"
#include "InputContext.h"
#include "MappedInput.h"

#include <queue>

class InputMapper;
struct NativeWindow;

using fnKeyStrokes_t = std::vector<flan::core::eInputKey>;

class InputReader
{
public:
    struct KeyEvent {
        flan::core::eInputKey   InputKey;
        int32_t                 IsDown; // Makes the struct 8 bytes aligned
    };

    struct AxisEvent {
        flan::core::eInputAxis  InputAxis;
        uint32_t                __PADDING__;
        double                  RawAxisValue;
    };

public:
                                InputReader();
                                InputReader( InputReader& ) = delete;
                                ~InputReader();

    void                        create();
    void                        onFrame( InputMapper* inputMapper );
    flan::core::eInputLayout    getActiveInputLayout() const;

    void                        pushKeyEvent( InputReader::KeyEvent&& keyEvent );
    void                        pushAxisEvent( InputReader::AxisEvent&& axisEvent );

    void                        pushKeyStroke( const flan::core::eInputKey keyStroke );
    fnKeyStrokes_t              getAndFlushKeyStrokes();

    void                        setAbsoluteAxisValue( const flan::core::eInputAxis axis, const double absoluteValue );
    double                      getAbsoluteAxisValue( const flan::core::eInputAxis axis );

private:
    flan::core::eInputLayout        activeInputLayout;
    std::queue<KeyEvent>            inputKeyEventQueue;
    std::queue<AxisEvent>           inputAxisEventQueue;

    fnKeyStrokes_t                  textInput;

    double                          absoluteAxisValues[flan::core::eInputAxis::InputAxis_COUNT];
    bool                            buttonMap[flan::core::KEYBOARD_KEY_COUNT];
};
