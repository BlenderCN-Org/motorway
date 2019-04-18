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

class BaseAllocator;
class DrawCommandBuilder;
class GUIWidget;
class GUIPanel;

#include <Maths/Vector.h>
#include <vector>

class GUIScreen
{
public:
                            GUIScreen( BaseAllocator* allocator );
                            GUIScreen( GUIScreen& widget ) = default;
                            GUIScreen& operator = ( GUIScreen& widget ) = default;
                            ~GUIScreen();

    void                    setVirtualScreenSize( const nyaVec2u& virtualScreenSizeInPixels );
    void                    onScreenResize( const nyaVec2u& screenSizeInPixels );
   
    GUIPanel&               allocatePanel();
    template<typename T> T* allocateWidget()
    {
        T* widget = nya::core::allocate<T>( memoryAllocator );
        widgets.push_back( widget );
        return widget;
    }

    void                    collectDrawCmds( DrawCommandBuilder& drawCmdBuilder );

    void                    onMouseCoordinatesUpdate( const double mouseX, const double mouseY );
    void                    onLeftMouseButtonDown( const double mouseX, const double mouseY );
    void                    onLeftMouseButtonUp();

private:
    BaseAllocator*          memoryAllocator;
    nyaVec2u                virtualScreenSize;
    nyaVec2u                screenSize;

    std::vector<GUIWidget*> widgets;
    std::vector<GUIPanel*>  panels;
};
