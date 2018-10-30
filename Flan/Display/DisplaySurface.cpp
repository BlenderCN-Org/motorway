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

#if FLAN_UNIX
#include "DisplaySurfaceXcb.h"
#include "EventLoopXcb.h"
#elif FLAN_WIN
#include "DisplaySurfaceWin32.h"
#include "EventLoopWin32.h"
#endif

using namespace flan::core;

DisplaySurface::DisplaySurface( const fnString_t& surfaceCaption )
    : nativeDisplaySurface( nullptr )
    , memoryAllocator( nullptr )
    , caption( surfaceCaption )
    , width( 640 )
    , height( 480 )
    , displayMode( eDisplayMode::WINDOWED )
    , isCursorVisible( true )
{

}

DisplaySurface::~DisplaySurface()
{
    nativeDisplaySurface = nullptr;
    memoryAllocator = nullptr;

    caption.clear();
    width = 0;
    height = 0;
    displayMode = eDisplayMode::WINDOWED;
    isCursorVisible = true;
}

void DisplaySurface::create( const uint32_t surfaceWidth, const uint32_t surfaceHeight, BaseAllocator* allocator )
{
    width = surfaceWidth;
    height = surfaceHeight;

    nativeDisplaySurface = CreateDisplaySurfaceImpl( width, height );

    if ( nativeDisplaySurface == nullptr ) {
        return;
    }

    SetDisplaySurfaceCaptionImpl( nativeDisplaySurface, caption );

    setDisplayMode( displayMode );
}

void DisplaySurface::setDisplayMode( const flan::core::eDisplayMode newDisplayMode )
{
    FLAN_CLOG << "Setting display mode from " << displayMode << " to " << newDisplayMode << std::endl;

    switch ( newDisplayMode ) {
    case FULLSCREEN:
        ToggleFullscreenImpl( nativeDisplaySurface );
        break;

    case BORDERLESS:
        ToggleBorderlessImpl( nativeDisplaySurface );
        break;

    case WINDOWED:
        // Toggle previous mode to get back to windowed mode
        if ( displayMode == FULLSCREEN ) {
            ToggleFullscreenImpl( nativeDisplaySurface );
        } else if ( displayMode == BORDERLESS ) {
            ToggleBorderlessImpl( nativeDisplaySurface );
        }
        break;

    default:
        break;
    }

    displayMode = newDisplayMode;
}

void DisplaySurface::pumpEvents( InputReader* inputReader ) const
{
    PollSystemEventsImpl( nativeDisplaySurface, inputReader );
}

bool DisplaySurface::shouldQuit() const
{
    return GetShouldQuitFlagImpl( nativeDisplaySurface );
}

flan::core::eDisplayMode DisplaySurface::getDisplayMode() const
{
    return displayMode;
}

NativeDisplaySurface* DisplaySurface::getNativeDisplaySurface() const
{
    return nativeDisplaySurface;
}

void DisplaySurface::getSurfaceDimension( uint32_t& surfaceWidth, uint32_t& surfaceHeight ) const
{
    surfaceWidth = width;
    surfaceHeight = height;
}

void DisplaySurface::setMousePosition( const float surfaceNormalizedX, const float surfaceNormalizedY )
{
    SetMousePositionImpl( nativeDisplaySurface, surfaceNormalizedX, surfaceNormalizedY );
}
