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
#include "Screen.h"

#include <Graphics/DrawCommandBuilder.h>

#include "Widget.h"
#include "Panel.h"

GUIScreen::GUIScreen( BaseAllocator* allocator )
    : memoryAllocator( allocator )
    , virtualScreenSize( 1280u, 720u )
    , screenSize( 1280u, 720u )
{

}

GUIScreen::~GUIScreen()
{
    memoryAllocator = nullptr;
}

void GUIScreen::setVirtualScreenSize( const nyaVec2u& virtualScreenSizeInPixels )
{
    virtualScreenSize = virtualScreenSize;
    virtualScreenSize = nya::maths::max( virtualScreenSize, nyaVec2u( 1u ) );
}

void GUIScreen::onScreenResize( const nyaVec2u& screenSizeInPixels )
{
    screenSize = screenSizeInPixels;

    nyaVec2f virtualToScreenRatio = nyaVec2f( screenSize ) / nyaVec2f( virtualScreenSize );
    for ( GUIPanel* panel : panels ) {
        panel->onScreenSizeChange( virtualToScreenRatio );
    } 

    for ( GUIWidget* widget : widgets ) {
        widget->onScreenSizeChange( virtualToScreenRatio );
    }
}

GUIPanel& GUIScreen::allocatePanel()
{
    nyaVec2f virtualToScreenRatio = nyaVec2f( screenSize ) / nyaVec2f( virtualScreenSize );

    GUIPanel* panel = nya::core::allocate<GUIPanel>( memoryAllocator );
    panels.push_back( panel );

    panel->onScreenSizeChange( virtualToScreenRatio );

    return *panel;
}

void GUIScreen::collectDrawCmds( DrawCommandBuilder& drawCmdBuilder )
{
    for ( GUIPanel* panel : panels ) {
        panel->collectDrawCmds( drawCmdBuilder );
    }

    for ( GUIWidget* widget : widgets ) {
        widget->collectDrawCmds( drawCmdBuilder );
    }
}

void GUIScreen::onMouseCoordinatesUpdate( const double mouseX, const double mouseY )
{
    for ( GUIPanel* panel : panels ) {
        panel->onMouseCoordinatesUpdate( mouseX, mouseY );
    }
}

void GUIScreen::onLeftMouseButtonDown( const double mouseX, const double mouseY )
{
    for ( GUIPanel* panel : panels ) {
        panel->onMouseButtonDown( mouseX, mouseY );
    }
}

void GUIScreen::onLeftMouseButtonUp()
{
    for ( GUIPanel* panel : panels ) {
        panel->onMouseButtonUp();
    }
}
