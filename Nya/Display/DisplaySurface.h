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
    namespace display
    {
        enum eDisplayMode : uint32_t;
    }
}
class BaseAllocator;
struct NativeDisplaySurface;
class InputReader;

struct DisplaySurface
{
    NativeDisplaySurface*       nativeDisplaySurface;
    BaseAllocator*              memoryAllocator;
    nya::display::eDisplayMode  displayMode;
};

namespace nya
{
    namespace display
    {
        DisplaySurface*     CreateDisplaySurface( BaseAllocator* allocator, const uint32_t surfaceWidth, const uint32_t surfaceHeight );
        void                DestroyDisplaySurface( DisplaySurface* surface );
        void                SetDisplayMode( DisplaySurface* surface, const eDisplayMode displayMode );
        void                PollSystemEvents( DisplaySurface* surface, InputReader* inputReader );
        void                SetCaption( DisplaySurface* surface, const nyaChar_t* caption );

        // NOTE Coordinates must be in range 0..1
        void                SetMousePosition( DisplaySurface* surface, const float x, const float y );

        bool                HasReceivedQuitSignal( DisplaySurface* surface );
    }
}
