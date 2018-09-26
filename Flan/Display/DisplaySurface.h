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

#include "DisplayMode.h"

struct NativeDisplaySurface;
class InputReader;
class DisplaySurface
{
public:
                                            DisplaySurface( const fnString_t& surfaceCaption );
                                            DisplaySurface( DisplaySurface& ) = default;
                                            DisplaySurface& operator = ( DisplaySurface& ) = default;
                                            ~DisplaySurface();

    void                                    create( const uint32_t surfaceWidth, const uint32_t surfaceHeight );
    void                                    pumpEvents( InputReader* inputReader ) const;
    void                                    setDisplayMode( const flan::core::eDisplayMode newDisplayMode );

    bool                                    shouldQuit() const;

    flan::core::eDisplayMode                getDisplayMode() const;
    NativeDisplaySurface*                   getNativeDisplaySurface() const;
    void                                    getSurfaceDimension( uint32_t& surfaceWidth, uint32_t& surfaceHeight ) const;

    void                                    setMousePosition( const float surfaceNormalizedX, const float surfaceNormalizedY );

private:
    std::unique_ptr<NativeDisplaySurface>   nativeDisplaySurface;
    fnString_t                              caption;
    uint32_t                                width;
    uint32_t                                height;
    flan::core::eDisplayMode                displayMode;
    bool                                    isCursorVisible;
};
