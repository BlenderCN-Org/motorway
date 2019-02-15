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
#include "DisplaySurface.h"

#if NYA_UNIX
#include "DisplaySurfaceXcb.h"
#include "EventLoopXcb.h"
#elif NYA_WIN
#include "DisplaySurfaceWin32.h"
#include "EventLoopWin32.h"
#endif

#include "DisplayMode.h"

using namespace nya::core;

DisplaySurface* nya::display::CreateDisplaySurface( BaseAllocator* allocator, const uint32_t surfaceWidth, const uint32_t surfaceHeight )
{
    DisplaySurface* displaySurface = nya::core::allocate<DisplaySurface>( allocator );
    displaySurface->nativeDisplaySurface = nya::display::CreateDisplaySurfaceImpl( allocator, surfaceWidth, surfaceHeight );
    displaySurface->memoryAllocator = allocator;
    displaySurface->displayMode = eDisplayMode::WINDOWED;

    return displaySurface;
}

void nya::display::DestroyDisplaySurface( DisplaySurface* surface )
{
    nya::core::free( surface->memoryAllocator, surface->nativeDisplaySurface );
    nya::core::free( surface->memoryAllocator, surface );
}

void nya::display::SetDisplayMode( DisplaySurface* surface, const eDisplayMode displayMode )
{
    switch ( displayMode ) {
    case FULLSCREEN:
        ToggleFullscreenImpl( surface->nativeDisplaySurface );
        break;

    case BORDERLESS:
        ToggleBorderlessImpl( surface->nativeDisplaySurface );
        break;

    case WINDOWED:
        // Toggle previous mode to get back to windowed mode
        if ( surface->displayMode == FULLSCREEN ) {
            ToggleFullscreenImpl( surface->nativeDisplaySurface );
        } else if ( surface->displayMode == BORDERLESS ) {
            ToggleBorderlessImpl( surface->nativeDisplaySurface );
        }
        break;

    default:
        break;
    }

    surface->displayMode = displayMode;
}

void nya::display::PollSystemEvents( DisplaySurface* surface, InputReader* inputReader )
{
    NYA_PROFILE( __FUNCTION__ )

    PollSystemEventsImpl( surface->nativeDisplaySurface, inputReader );
}

void nya::display::SetCaption( DisplaySurface* surface, const nyaChar_t* caption )
{
    SetDisplaySurfaceCaptionImpl( surface->nativeDisplaySurface, caption );
}

void nya::display::SetMousePosition( DisplaySurface* surface, const float x, const float y )
{
    SetMousePositionImpl( surface->nativeDisplaySurface, x, y );
}

bool nya::display::HasReceivedQuitSignal( DisplaySurface* surface )
{
    return GetShouldQuitFlagImpl( surface->nativeDisplaySurface );
}
